package com.quantatissu.orchestrator.service;

import org.junit.jupiter.api.Test;
import java.io.IOException;
import java.util.Map;
import static org.junit.jupiter.api.Assertions.*;

public class ProcessManagerTest {

    @Test
    public void testStartAndStopProcess() throws IOException, InterruptedException {
        ProcessManager manager = new ProcessManager();
        String taskId = "test-task";
        String command = "sleep 10";

        String id = manager.startProcess(taskId, command, null);
        assertEquals(taskId, id);

        ProcessManager.ProcessInfo info = manager.getProcessInfo(taskId);
        assertNotNull(info);
        assertEquals("RUNNING", info.status);

        boolean stopped = manager.stopProcess(taskId);
        assertTrue(stopped);
        assertTrue("STOPPED".equals(info.status) || "FAILED".equals(info.status));
    }

    @Test
    public void testProcessLogAccumulation() throws IOException, InterruptedException {
        ProcessManager manager = new ProcessManager();
        String taskId = "log-task";
        // Use a command that produces output quickly
        String command = "echo hello-world";

        manager.startProcess(taskId, command, null);

        // Wait a bit for the process to finish and logs to be read
        Thread.sleep(2000);

        ProcessManager.ProcessInfo info = manager.getProcessInfo(taskId);
        assertNotNull(info);
        assertTrue(info.logs.size() > 0);
        assertTrue(info.logs.contains("hello-world"));
        assertTrue("COMPLETED".equals(info.status) || "FAILED".equals(info.status));
    }

    @Test
    public void testVerificationData() throws IOException {
        ProcessManager manager = new ProcessManager();
        String taskId = "verify-task";
        manager.startProcess(taskId, "sleep 5", null);

        Map<String, Object> verification = manager.getVerification(taskId);
        assertEquals(true, verification.get("verified"));
        assertEquals("RUNNING", verification.get("status"));

        manager.stopProcess(taskId);
    }
}
