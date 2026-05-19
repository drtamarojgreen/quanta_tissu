package com.quantatissu.orchestrator.controller;

import com.quantatissu.orchestrator.model.ProcessTask;
import com.quantatissu.orchestrator.service.TaskService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;
import java.io.IOException;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

@RestController
@RequestMapping("/api/orchestrator")
public class OrchestratorController {

    @Autowired
    private TaskService taskService;

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

    @GetMapping("/processes")
    public ResponseEntity<List<ProcessTask>> listProcesses() {
        return ResponseEntity.ok(taskService.getAllTasks());
    }

    @PostMapping("/processes")
    public ResponseEntity<ProcessTask> startProcess(@RequestBody Map<String, String> payload) throws IOException {
        String taskId = payload.get("task_id");
        String type = payload.get("type");
        String command = payload.get("command");
        String workingDir = payload.get("working_dir");

        ProcessTask task = taskService.startProcess(taskId, type, command, workingDir);
        return ResponseEntity.ok(task);
    }

    @GetMapping("/processes/{taskId}")
    public ResponseEntity<ProcessTask> getProcessStatus(@PathVariable String taskId) {
        ProcessTask task = taskService.getTaskStatus(taskId);
        if (task == null) return ResponseEntity.notFound().build();
        return ResponseEntity.ok(task);
    }

    @GetMapping("/processes/{taskId}/logs")
    public ResponseEntity<Map<String, Object>> getProcessLogs(@PathVariable String taskId, @RequestParam(defaultValue = "0") int cursor) {
        List<String> logs = taskService.getLogs(taskId, cursor);
        Map<String, Object> response = new HashMap<>();
        response.put("logs", logs);
        response.put("cursor", cursor + logs.size());
        return ResponseEntity.ok(response);
    }

    @DeleteMapping("/processes/{taskId}")
    public ResponseEntity<Map<String, String>> stopProcess(@PathVariable String taskId) {
        taskService.stopProcess(taskId);
        Map<String, String> response = new HashMap<>();
        response.put("message", "Process stop signal sent");
        return ResponseEntity.ok(response);
    }

    @GetMapping("/processes/verify/{taskId}")
    public ResponseEntity<Map<String, Object>> verifyProcess(@PathVariable String taskId) {
        ProcessTask task = taskService.getTaskStatus(taskId);
        Map<String, Object> result = new HashMap<>();
        if (task == null) {
            result.put("valid", false);
            result.put("error", "Task not found");
            return ResponseEntity.ok(result);
        }

        boolean logContinuity = task.getLogs().size() > 0; // Simple check for now
        result.put("valid", true);
        result.put("taskId", taskId);
        result.put("state", task.getState());
        result.put("log_count", task.getLogs().size());
        result.put("log_continuity", logContinuity);

        return ResponseEntity.ok(result);
    }
}
