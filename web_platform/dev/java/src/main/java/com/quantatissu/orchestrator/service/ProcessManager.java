package com.quantatissu.orchestrator.service;

import org.springframework.stereotype.Service;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.*;
import java.util.concurrent.*;

@Service
public class ProcessManager {
    private final Map<String, ProcessInfo> processes = new ConcurrentHashMap<>();
    private final ExecutorService executor = Executors.newCachedThreadPool();

    public static class ProcessInfo {
        public String id;
        public String command;
        public String status;
        public long startTime;
        public long endTime;
        public List<String> logs = Collections.synchronizedList(new ArrayList<>());
        public Integer pid;
        private Process process;

        public Map<String, Object> toMap() {
            Map<String, Object> map = new HashMap<>();
            map.put("id", id);
            map.put("command", command);
            map.put("status", status);
            map.put("startTime", startTime);
            map.put("endTime", endTime);
            map.put("pid", pid);
            synchronized (logs) {
                map.put("logs", new ArrayList<>(logs.subList(Math.max(0, logs.size() - 50), logs.size())));
            }
            return map;
        }
    }

    private static final Set<String> ALLOWED_COMMANDS = Set.of("./analyzer", "python3", "echo", "sleep", "make", "ls");

    public String startProcess(String taskId, String command, String workingDir) throws IOException {
        if (processes.containsKey(taskId) && "RUNNING".equals(processes.get(taskId).status)) {
            throw new IOException("Process already running with ID: " + taskId);
        }

        String[] parts = command.split(" ");
        if (parts.length == 0 || !ALLOWED_COMMANDS.contains(parts[0])) {
            throw new SecurityException("Command not allowed: " + parts[0]);
        }

        ProcessBuilder pb = new ProcessBuilder(parts);
        if (workingDir != null) {
            pb.directory(new java.io.File(workingDir));
        }
        pb.redirectErrorStream(true);

        Process process = pb.start();
        ProcessInfo info = new ProcessInfo();
        info.id = taskId;
        info.command = command;
        info.status = "RUNNING";
        info.startTime = System.currentTimeMillis();
        info.process = process;

        try {
            java.lang.reflect.Field field = process.getClass().getDeclaredField("pid");
            field.setAccessible(true);
            info.pid = (Integer) field.get(process);
        } catch (Exception e) {
            // Fallback for non-Unix or restricted environments if necessary
        }

        processes.put(taskId, info);

        executor.submit(() -> {
            try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    info.logs.add(line);
                    if (info.logs.size() > 1000) {
                        info.logs.remove(0);
                    }
                }
                int exitCode = process.waitFor();
                info.status = (exitCode == 0) ? "COMPLETED" : "FAILED";
                info.endTime = System.currentTimeMillis();
            } catch (Exception e) {
                info.status = "ERROR";
                info.logs.add("Error reading process output: " + e.getMessage());
                info.endTime = System.currentTimeMillis();
            }
        });

        return taskId;
    }

    public boolean stopProcess(String taskId) {
        ProcessInfo info = processes.get(taskId);
        if (info == null || !"RUNNING".equals(info.status)) {
            return false;
        }
        info.process.destroy();
        try {
            if (!info.process.waitFor(5, TimeUnit.SECONDS)) {
                info.process.destroyForcibly();
            }
        } catch (InterruptedException e) {
            info.process.destroyForcibly();
        }
        info.status = "STOPPED";
        info.endTime = System.currentTimeMillis();
        return true;
    }

    public ProcessInfo getProcessInfo(String taskId) {
        return processes.get(taskId);
    }

    public Map<String, ProcessInfo> getAllProcesses() {
        return processes;
    }

    public Map<String, Object> getVerification(String taskId) {
        ProcessInfo info = processes.get(taskId);
        Map<String, Object> verification = new HashMap<>();
        if (info == null) {
            verification.put("verified", false);
            verification.put("reason", "Process not found");
            return verification;
        }
        verification.put("verified", true);
        verification.put("status", info.status);
        verification.put("logSize", info.logs.size());
        verification.put("hasPid", info.pid != null);
        verification.put("uptime", info.status.equals("RUNNING") ? (System.currentTimeMillis() - info.startTime) : (info.endTime - info.startTime));
        return verification;
    }
}
