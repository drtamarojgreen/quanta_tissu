package com.quantatissu.orchestrator.service;

import org.springframework.stereotype.Service;
import org.springframework.web.client.RestTemplate;
import org.springframework.http.ResponseEntity;

@Service
public class HttpServiceCall {

    private final RestTemplate restTemplate;

    public HttpServiceCall() {
        this.restTemplate = new RestTemplate();
    }

    public String get(String url) {
        try {
            ResponseEntity<String> response = restTemplate.getForEntity(url, String.class);
            return response.getBody();
        } catch (Exception e) {
            System.err.println("Error making GET request to " + url + ": " + e.getMessage());
            return null;
        }
    }

    public String post(String url, Object payload) {
        try {
            ResponseEntity<String> response = restTemplate.postForEntity(url, payload, String.class);
            return response.getBody();
        } catch (Exception e) {
            System.err.println("Error making POST request to " + url + ": " + e.getMessage());
            return null;
        }
    }
}
