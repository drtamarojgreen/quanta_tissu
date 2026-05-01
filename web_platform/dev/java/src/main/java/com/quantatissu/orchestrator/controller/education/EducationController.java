package com.quantatissu.orchestrator.controller.education;

import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/api/education")
public class EducationController {

    @GetMapping("/status")
    public String getStatus() {
        return "Education module is active.";
    }
}
