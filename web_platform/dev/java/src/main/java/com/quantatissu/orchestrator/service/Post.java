package com.quantatissu.orchestrator.service;

import org.springframework.stereotype.Service;

@Service
public class Post {
    public String sendPostRequest(String url, String data) {
        return "Post response from " + url;
    }
}
