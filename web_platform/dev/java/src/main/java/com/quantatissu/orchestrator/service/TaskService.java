package com.quantatissu.orchestrator.service;

import com.quantatissu.orchestrator.model.ProcessTask;
import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.HttpClients;
import org.apache.http.util.EntityUtils;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Service;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.time.LocalDateTime;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.List;
import java.util.ArrayList;

@Service
public class TaskService {

    @Value("${PLATFORM_HOST:localhost}")
    private String platformHost;

    @Value("${PLATFORM_PORT:8000}")
    private String platformPort;

    private final Map<String, ProcessTask> tasks = new ConcurrentHashMap<>();
    private final Map<String, Process> processes = new ConcurrentHashMap<>();

    public String listTasks() throws IOException {
        // Call internal Python endpoint to avoid recursion
        String url = String.format("http://%s:%s/api/internal/tasks", platformHost, platformPort);
        try (CloseableHttpClient httpClient = HttpClients.createDefault()) {
            HttpGet request = new HttpGet(url);
            try (CloseableHttpResponse response = httpClient.execute(request)) {
                return EntityUtils.toString(response.getEntity());
            }
        }
    }

    public ProcessTask startProcess(String taskId, String type, String command, String workingDir) throws IOException {
        if (tasks.containsKey(taskId) && tasks.get(taskId).getState() == ProcessTask.State.RUNNING) {
            throw new IllegalStateException("Task already running: " + taskId);
        }

        ProcessTask task = new ProcessTask(taskId, type, command);
        tasks.put(taskId, task);

        // Robust command splitting (simple shlex-like for now, handles spaces better than split(" "))
        List<String> commandList = parseCommand(command);
        ProcessBuilder pb = new ProcessBuilder(commandList);
        if (workingDir != null) {
            pb.directory(new java.io.File(workingDir));
        }
        pb.redirectErrorStream(true);

        Process process = pb.start();
        processes.put(taskId, process);

        task.setState(ProcessTask.State.RUNNING);
        task.setStartedAt(LocalDateTime.now());

        // Use Process.pid() available in Java 9+
        task.setPid((int) process.pid());

        new Thread(() -> {
            try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
                String line;
                while ((line = reader.readLine()) != null) {
                    task.addLog(line);
                }
            } catch (IOException e) {
                task.addLog("Error reading logs: " + e.getMessage());
            } finally {
                try {
                    int exitCode = process.waitFor();
                    task.setState(exitCode == 0 ? ProcessTask.State.COMPLETED : ProcessTask.State.FAILED);
                } catch (InterruptedException e) {
                    task.setState(ProcessTask.State.FAILED);
                    Thread.currentThread().interrupt();
                }
                task.setEndedAt(LocalDateTime.now());
                processes.remove(taskId);
            }
        }).start();

        return task;
    }

    private List<String> parseCommand(String command) {
        List<String> list = new ArrayList<>();
        StringBuilder sb = new StringBuilder();
        boolean inQuotes = false;
        for (char c : command.toCharArray()) {
            if (c == '\"') {
                inQuotes = !inQuotes;
            } else if (c == ' ' && !inQuotes) {
                if (sb.length() > 0) {
                    list.add(sb.toString());
                    sb.setLength(0);
                }
            } else {
                sb.append(c);
            }
        }
        if (sb.length() > 0) list.add(sb.toString());
        return list;
    }

    public void stopProcess(String taskId) {
        Process process = processes.get(taskId);
        if (process != null) {
            process.destroy();
            ProcessTask task = tasks.get(taskId);
            if (task != null) {
                task.setState(ProcessTask.State.STOPPED);
                task.setEndedAt(LocalDateTime.now());
            }
            processes.remove(taskId);
        }
    }

    public ProcessTask getTaskStatus(String taskId) {
        return tasks.get(taskId);
    }

    public List<String> getLogs(String taskId, int cursor) {
        ProcessTask task = tasks.get(taskId);
        if (task == null) return new ArrayList<>();
        List<String> allLogs = task.getLogs();
        if (cursor >= allLogs.size()) return new ArrayList<>();
        return allLogs.subList(cursor, allLogs.size());
    }

    public List<ProcessTask> getAllTasks() {
        return new ArrayList<>(tasks.values());
    }
}
