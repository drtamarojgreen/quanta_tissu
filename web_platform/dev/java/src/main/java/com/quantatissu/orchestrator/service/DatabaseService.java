package com.quantatissu.orchestrator.service;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.stereotype.Service;
import javax.annotation.PostConstruct;

@Service
public class DatabaseService {

    @Autowired
    private JdbcTemplate jdbcTemplate;

    @PostConstruct
    public void init() {
        startDatabase();
    }

    public void startDatabase() {
        // Initialize tables from the xmlcms2 model
        runSQLQuery("CREATE TABLE IF NOT EXISTS rooms (\n"
                + " id integer PRIMARY KEY,\n"
                + " sdp text,\n"
                + " type text,\n"
                + " attendees integer);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS article (\n"
                + " id integer PRIMARY KEY,\n"
                + " author text NOT NULL,\n"
                + " authorDate text NOT NULL,\n"
                + " title text NOT NULL,\n"
                + " description text NOT NULL,\n"
                + " content text NOT NULL);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS blog (\n"
                + " id integer PRIMARY KEY,\n"
                + " author text NOT NULL,\n"
                + " authorDate text NOT NULL,\n"
                + " title text NOT NULL,\n"
                + " description text NOT NULL,\n"
                + " content text NOT NULL);");

        // Add more tables as needed from the xmlcms2 model
    }

    public void runSQLQuery(String sql) {
        try {
            jdbcTemplate.execute(sql);
        } catch (Exception e) {
            System.err.println("Error executing SQL: " + e.getMessage());
        }
    }
}
