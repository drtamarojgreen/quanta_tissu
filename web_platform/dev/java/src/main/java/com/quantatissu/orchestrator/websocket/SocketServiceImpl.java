package com.quantatissu.orchestrator.websocket;

import org.springframework.stereotype.Service;

@Service
public class SocketServiceImpl {
    public void broadcastMessage(String message) {
        System.out.println("Broadcasting: " + message);
    }
}
