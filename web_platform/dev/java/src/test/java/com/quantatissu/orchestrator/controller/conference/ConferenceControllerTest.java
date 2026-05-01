package com.quantatissu.orchestrator.controller.conference;

import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.WebMvcTest;
import org.springframework.test.web.servlet.MockMvc;

import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.*;

@WebMvcTest(controllers = ConferenceController.class)
@org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc(addFilters = false)
public class ConferenceControllerTest {

    @Autowired
    private MockMvc mockMvc;

    @Test
    public void testShowConference() throws Exception {
        mockMvc.perform(get("/conferences/conference"))
                .andExpect(status().isOk())
                .andExpect(view().name("conferences/conference"))
                .andExpect(model().attributeExists("message"))
                .andExpect(model().attributeExists("conferenceName"));
    }

    @Test
    public void testShowRoom() throws Exception {
        mockMvc.perform(get("/conferences/room"))
                .andExpect(status().isOk())
                .andExpect(view().name("conferences/room"))
                .andExpect(model().attributeExists("rooms"));
    }

    @Test
    public void testShowPresentation() throws Exception {
        mockMvc.perform(get("/conferences/presentation"))
                .andExpect(status().isOk())
                .andExpect(view().name("conferences/presentation"));
    }
}
