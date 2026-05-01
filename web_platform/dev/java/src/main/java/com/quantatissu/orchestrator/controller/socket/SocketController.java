package com.quantatissu.orchestrator.controller.socket;

import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/api/socket")
public class SocketController {

    @GetMapping("/status")
    public String getStatus() {
        return "Socket module is active.";
    }
}
