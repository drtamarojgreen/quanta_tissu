package com.quantatissu.orchestrator.controller;

import com.quantatissu.orchestrator.service.TaskService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;
import java.io.IOException;
import java.util.HashMap;
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
}
