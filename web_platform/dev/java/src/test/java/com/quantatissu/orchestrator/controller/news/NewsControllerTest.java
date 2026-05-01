package com.quantatissu.orchestrator.controller.news;

import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.WebMvcTest;
import org.springframework.test.web.servlet.MockMvc;

import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.view;

@WebMvcTest(controllers = NewsController.class)
@org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc(addFilters = false)
public class NewsControllerTest {

    @Autowired private MockMvc mockMvc;

    @Test public void testShowReports() throws Exception { mockMvc.perform(get("/news/reports")).andExpect(status().isOk()).andExpect(view().name("news/reports")); }
    @Test public void testShowForum() throws Exception { mockMvc.perform(get("/news/forum")).andExpect(status().isOk()).andExpect(view().name("news/forum")); }
    @Test public void testShowNews() throws Exception { mockMvc.perform(get("/news/news")).andExpect(status().isOk()).andExpect(view().name("news/news")); }
    @Test public void testShowFreelance() throws Exception { mockMvc.perform(get("/news/freelance")).andExpect(status().isOk()).andExpect(view().name("news/freelance")); }
}
