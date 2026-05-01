package com.quantatissu.orchestrator.service;

import org.springframework.stereotype.Service;
import java.nio.file.Files;
import java.nio.file.Paths;

@Service
public class XmlRetriever {

    public String retrieveFromFile(String filePath) {
        try {
            return new String(Files.readAllBytes(Paths.get(filePath)));
        } catch (Exception e) {
            System.err.println("Error reading XML from file " + filePath + ": " + e.getMessage());
            return null;
        }
    }
}
