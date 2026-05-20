package com.quantatissu.orchestrator.controller;

import com.quantatissu.orchestrator.service.ProcessManager;
import com.quantatissu.orchestrator.service.TaskService;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.WebMvcTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.http.MediaType;
import org.springframework.test.web.servlet.MockMvc;

import java.util.HashMap;
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

    @MockBean
    private ProcessManager processManager;

    @Test
    public void testGetStatus() throws Exception {
        mockMvc.perform(get("/api/orchestrator/status"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.status").value("online"));
    }

    @Test
    public void testStartProcess() throws Exception {
        when(processManager.startProcess(eq("task-1"), eq("echo test"), eq("/tmp")))
                .thenReturn("task-1");

        String json = "{\"taskId\":\"task-1\", \"command\":\"echo test\", \"workingDir\":\"/tmp\"}";

        mockMvc.perform(post("/api/orchestrator/processes")
                .contentType(MediaType.APPLICATION_JSON)
                .content(json))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.success").value(true))
                .andExpect(jsonPath("$.taskId").value("task-1"));
    }

    @Test
    public void testGetProcess() throws Exception {
        ProcessManager.ProcessInfo info = new ProcessManager.ProcessInfo();
        info.id = "task-1";
        info.status = "RUNNING";

        when(processManager.getProcessInfo("task-1")).thenReturn(info);

        mockMvc.perform(get("/api/orchestrator/processes/task-1"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.id").value("task-1"))
                .andExpect(jsonPath("$.status").value("RUNNING"));
    }

    @Test
    public void testStopProcess() throws Exception {
        when(processManager.stopProcess("task-1")).thenReturn(true);

        mockMvc.perform(delete("/api/orchestrator/processes/task-1"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.success").value(true));
    }

    @Test
    public void testVerifyProcess() throws Exception {
        Map<String, Object> verification = new HashMap<>();
        verification.put("verified", true);

        when(processManager.getVerification("task-1")).thenReturn(verification);

        mockMvc.perform(get("/api/orchestrator/processes/verify/task-1"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.verified").value(true));
    }
}
