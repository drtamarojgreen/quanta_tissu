package com.quantatissu.orchestrator.controller;

import com.quantatissu.orchestrator.service.ProcessManager;
import com.quantatissu.orchestrator.service.TaskService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.stream.Collectors;

@RestController
@RequestMapping("/api/orchestrator")
public class OrchestratorController {

    @Autowired
    private TaskService taskService;

    @Autowired
    private ProcessManager processManager;

    @GetMapping("/status")
    public Map<String, String> getStatus() {
        Map<String, String> status = new HashMap<>();
        status.put("status", "online");
        status.put("message", "QuantaTissu Java Orchestrator is running.");
        return status;
    }

    @GetMapping("/tasks")
    public String getTasks() throws IOException {
        return taskService.listTasks();
    }

    @PostMapping("/processes")
    public ResponseEntity<Map<String, Object>> startProcess(@RequestBody Map<String, String> request) {
        String taskId = request.get("taskId");
        String command = request.get("command");
        String workingDir = request.get("workingDir");

        try {
            processManager.startProcess(taskId, command, workingDir);
            Map<String, Object> response = new HashMap<>();
            response.put("success", true);
            response.put("taskId", taskId);
            return ResponseEntity.ok(response);
        } catch (IOException e) {
            Map<String, Object> error = new HashMap<>();
            error.put("success", false);
            error.put("error", e.getMessage());
            return ResponseEntity.badRequest().body(error);
        }
    }

    @GetMapping("/processes")
    public Map<String, Object> listProcesses() {
        return processManager.getAllProcesses().entrySet().stream()
                .collect(Collectors.toMap(Map.Entry::getKey, e -> e.getValue().toMap()));
    }

    @GetMapping("/processes/{taskId}")
    public ResponseEntity<Map<String, Object>> getProcess(@PathVariable String taskId) {
        ProcessManager.ProcessInfo info = processManager.getProcessInfo(taskId);
        if (info == null) {
            return ResponseEntity.notFound().build();
        }
        return ResponseEntity.ok(info.toMap());
    }

    @DeleteMapping("/processes/{taskId}")
    public ResponseEntity<Map<String, Object>> stopProcess(@PathVariable String taskId) {
        boolean success = processManager.stopProcess(taskId);
        Map<String, Object> response = new HashMap<>();
        response.put("success", success);
        return ResponseEntity.ok(response);
    }

    @GetMapping("/processes/verify/{taskId}")
    public Map<String, Object> verifyProcess(@PathVariable String taskId) {
        return processManager.getVerification(taskId);
    }
}
