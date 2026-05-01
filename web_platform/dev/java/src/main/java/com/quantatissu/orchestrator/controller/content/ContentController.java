package com.quantatissu.orchestrator.controller.content;

import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/api/content")
public class ContentController {

    @GetMapping("/status")
    public String getStatus() {
        return "Content module is active.";
    }
}
