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

                runSQLQuery("CREATE TABLE IF NOT EXISTS review (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS journal (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS subscription (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS shopcart (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS shoporder (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS shopproduct (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS shopitem (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS shoppayment (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS school (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS course (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS lecture (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS lecturer (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS student (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS artificialintelligence (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS machinelearning (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS aiml (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS hrgroup (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS researcher (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS project (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS topic (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS accountuser (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS userdetails (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS configuration (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS chat (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS event (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS video (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS artist (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

                runSQLQuery("CREATE TABLE IF NOT EXISTS cartitem (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS shopmessage (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS shopordercart (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS cartstatus (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS shoppaymenttype (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS shoporderstatus (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS administrator (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS administratorgroup (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS lecturenote (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS hrclient (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS hremployer (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS eventadministrator (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS eventadvertisement (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS social (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS publisher (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS game (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS socialmedia (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS bookpage (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS consulting (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS dbconfig (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS displaymap (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS fileupload (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS login (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS messagehandler (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS sseemitters (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS market (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS rssfeed (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS socketclient (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS socketmessage (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS socketrequest (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS socketsession (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS socketsubscriber (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

        runSQLQuery("CREATE TABLE IF NOT EXISTS socketsubscription (\n"
                + " id integer PRIMARY KEY,\n"
                + " name text,\n"
                + " description text);");

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
