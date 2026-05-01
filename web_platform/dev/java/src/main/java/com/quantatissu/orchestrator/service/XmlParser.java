package com.quantatissu.orchestrator.service;

import org.springframework.stereotype.Service;
import org.w3c.dom.Document;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import java.io.ByteArrayInputStream;

@Service
public class XmlParser {

    public Document parseXmlString(String xmlStr) {
        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            DocumentBuilder builder = factory.newDocumentBuilder();
            ByteArrayInputStream input = new ByteArrayInputStream(xmlStr.getBytes("UTF-8"));
            return builder.parse(input);
        } catch (Exception e) {
            System.err.println("Error parsing XML: " + e.getMessage());
            return null;
        }
    }
}
