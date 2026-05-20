package com.quantatissu.orchestrator.model;

import java.time.LocalDateTime;
import java.util.ArrayList;
import java.util.List;

public class ProcessTask {
    public enum State {
        PENDING,
        RUNNING,
        COMPLETED,
        FAILED,
        STOPPED
    }

    private String taskId;
    private String type;
    private String command;
    private State state;
    private LocalDateTime createdAt;
    private LocalDateTime startedAt;
    private LocalDateTime endedAt;
    private Integer pid;
    private final List<String> logs = new ArrayList<>();

    public ProcessTask(String taskId, String type, String command) {
        this.taskId = taskId;
        this.type = type;
        this.command = command;
        this.state = State.PENDING;
        this.createdAt = LocalDateTime.now();
    }

    // Getters and Setters
    public String getTaskId() { return taskId; }
    public void setTaskId(String taskId) { this.taskId = taskId; }

    public String getType() { return type; }
    public void setType(String type) { this.type = type; }

    public String getCommand() { return command; }
    public void setCommand(String command) { this.command = command; }

    public State getState() { return state; }
    public void setState(State state) { this.state = state; }

    public LocalDateTime getCreatedAt() { return createdAt; }
    public void setCreatedAt(LocalDateTime createdAt) { this.createdAt = createdAt; }

    public LocalDateTime getStartedAt() { return startedAt; }
    public void setStartedAt(LocalDateTime startedAt) { this.startedAt = startedAt; }

    public LocalDateTime getEndedAt() { return endedAt; }
    public void setEndedAt(LocalDateTime endedAt) { this.endedAt = endedAt; }

    public Integer getPid() { return pid; }
    public void setPid(Integer pid) { this.pid = pid; }

    public List<String> getLogs() {
        synchronized (logs) {
            return new ArrayList<>(logs);
        }
    }

    public void addLog(String log) {
        synchronized (logs) {
            this.logs.add(log);
            if (this.logs.size() > 2000) {
                this.logs.remove(0);
            }
        }
    }
}
