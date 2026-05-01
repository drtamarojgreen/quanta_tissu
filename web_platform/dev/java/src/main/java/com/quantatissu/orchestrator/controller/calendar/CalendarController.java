package com.quantatissu.orchestrator.controller.calendar;

import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/api/calendar")
public class CalendarController {

    @GetMapping("/status")
    public String getStatus() {
        return "Calendar module is active.";
    }
}
