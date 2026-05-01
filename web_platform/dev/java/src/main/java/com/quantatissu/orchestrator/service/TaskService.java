package com.quantatissu.orchestrator.service;

import org.apache.http.client.methods.CloseableHttpResponse;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.HttpClients;
import org.apache.http.util.EntityUtils;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Service;

import java.io.IOException;

@Service
public class TaskService {

    @Value("${PLATFORM_HOST:localhost}")
    private String platformHost;

    @Value("${PLATFORM_PORT:8000}")
    private String platformPort;

    public String listTasks() throws IOException {
        String url = String.format("http://%s:%s/api/tasks", platformHost, platformPort);
        try (CloseableHttpClient httpClient = HttpClients.createDefault()) {
            HttpGet request = new HttpGet(url);
            try (CloseableHttpResponse response = httpClient.execute(request)) {
                return EntityUtils.toString(response.getEntity());
            }
        }
    }
}
