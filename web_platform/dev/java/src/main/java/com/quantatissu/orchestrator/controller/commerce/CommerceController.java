package com.quantatissu.orchestrator.controller.commerce;

import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/api/commerce")
public class CommerceController {

    @GetMapping("/status")
    public String getStatus() {
        return "Commerce module is active.";
    }
}
