package com.quantatissu.orchestrator.controller.intelligence;

import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/api/intelligence")
public class IntelligenceController {

    @GetMapping("/status")
    public String getStatus() {
        return "Intelligence module is active.";
    }
}
