package com.quantatissu.orchestrator.service;

import org.springframework.stereotype.Service;

@Service
public class FormValidation {
    public boolean isValid(Object form) {
        return form != null;
    }
}
