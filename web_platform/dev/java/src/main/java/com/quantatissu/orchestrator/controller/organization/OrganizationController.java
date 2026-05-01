package com.quantatissu.orchestrator.controller.organization;

import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/api/organization")
public class OrganizationController {

    @GetMapping("/status")
    public String getStatus() {
        return "Organization module is active.";
    }
}
