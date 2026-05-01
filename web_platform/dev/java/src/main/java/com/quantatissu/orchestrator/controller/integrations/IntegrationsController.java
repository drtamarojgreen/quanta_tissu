package com.quantatissu.orchestrator.controller.integrations;

import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/api/integrations")
public class IntegrationsController {

    @GetMapping("/status")
    public String getStatus() {
        return "Integrations module is active.";
    }
}
