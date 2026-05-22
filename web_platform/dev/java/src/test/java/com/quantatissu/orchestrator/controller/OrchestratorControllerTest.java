package com.quantatissu.orchestrator.controller;

import com.quantatissu.orchestrator.model.ProcessTask;
import com.quantatissu.orchestrator.service.TaskService;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.WebMvcTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.http.MediaType;
import org.springframework.test.web.servlet.MockMvc;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.when;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.*;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.*;

@WebMvcTest(controllers = OrchestratorController.class)
@org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc(addFilters = false)
public class OrchestratorControllerTest {

    @Autowired
    private MockMvc mockMvc;

    @MockBean
    private TaskService taskService;

    @Test
    public void testGetStatus() throws Exception {
        mockMvc.perform(get("/api/orchestrator/status"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.status").value("online"));
    }

    @Test
    public void testStartProcess() throws Exception {
        ProcessTask task = new ProcessTask("task-1", "analyzer", "echo test");
        when(taskService.startProcess(eq("task-1"), eq("analyzer"), eq("echo test"), eq("/tmp")))
                .thenReturn(task);

        String json = "{\"task_id\":\"task-1\", \"type\":\"analyzer\", \"command\":\"echo test\", \"working_dir\":\"/tmp\"}";

        mockMvc.perform(post("/api/orchestrator/processes")
                .contentType(MediaType.APPLICATION_JSON)
                .content(json))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.taskId").value("task-1"))
                .andExpect(jsonPath("$.state").value("PENDING"));
    }

    @Test
    public void testGetProcess() throws Exception {
        ProcessTask task = new ProcessTask("task-1", "analyzer", "echo test");
        task.setState(ProcessTask.State.RUNNING);

        when(taskService.getTaskStatus("task-1")).thenReturn(task);

        mockMvc.perform(get("/api/orchestrator/processes/task-1"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.taskId").value("task-1"))
                .andExpect(jsonPath("$.state").value("RUNNING"));
    }

    @Test
    public void testStopProcess() throws Exception {
        mockMvc.perform(delete("/api/orchestrator/processes/task-1"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.message").value("Process stop signal sent"));
    }

    @Test
    public void testVerifyProcess() throws Exception {
        ProcessTask task = new ProcessTask("task-1", "analyzer", "echo test");
        task.setState(ProcessTask.State.RUNNING);
        task.addLog("Started");

        when(taskService.getTaskStatus("task-1")).thenReturn(task);

        mockMvc.perform(get("/api/orchestrator/processes/verify/task-1"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.valid").value(true))
                .andExpect(jsonPath("$.taskId").value("task-1"))
                .andExpect(jsonPath("$.state").value("RUNNING"))
                .andExpect(jsonPath("$.log_count").value(1))
                .andExpect(jsonPath("$.log_continuity").value(true));
    }
}
