package com.quantatissu.orchestrator.filter;

import org.springframework.stereotype.Component;
import javax.annotation.PostConstruct;

@Component
public class ApplicationInitializer {
    @PostConstruct
    public void init() {
        System.out.println("Application Initialized.");
    }
}
