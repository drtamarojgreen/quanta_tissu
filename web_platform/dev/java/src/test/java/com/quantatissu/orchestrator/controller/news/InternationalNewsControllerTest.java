package com.quantatissu.orchestrator.controller.news;

import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.WebMvcTest;
import org.springframework.test.web.servlet.MockMvc;

import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.view;

@WebMvcTest(InternationalNewsController.class)
public class InternationalNewsControllerTest {

    @Autowired private MockMvc mockMvc;

    @Test public void testShowBusiness() throws Exception { mockMvc.perform(get("/international/business")).andExpect(status().isOk()).andExpect(view().name("international/business")); }
    @Test public void testShowFinance() throws Exception { mockMvc.perform(get("/international/finance")).andExpect(status().isOk()).andExpect(view().name("international/finance")); }
    @Test public void testShowLocal() throws Exception { mockMvc.perform(get("/international/local")).andExpect(status().isOk()).andExpect(view().name("international/local")); }
    @Test public void testShowNews() throws Exception { mockMvc.perform(get("/international/news")).andExpect(status().isOk()).andExpect(view().name("international/news")); }
    @Test public void testShowPolitics() throws Exception { mockMvc.perform(get("/international/politics")).andExpect(status().isOk()).andExpect(view().name("international/politics")); }
    @Test public void testShowInternational() throws Exception { mockMvc.perform(get("/international/international")).andExpect(status().isOk()).andExpect(view().name("international/international")); }
}
