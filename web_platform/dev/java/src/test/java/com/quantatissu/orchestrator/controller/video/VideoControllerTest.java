package com.quantatissu.orchestrator.controller.video;

import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.WebMvcTest;
import org.springframework.test.web.servlet.MockMvc;

import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.post;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.view;

@WebMvcTest(controllers = VideoController.class)
@org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc(addFilters = false)
public class VideoControllerTest {

    @Autowired private MockMvc mockMvc;

    @Test public void testVideoList() throws Exception { mockMvc.perform(get("/videos/videoList")).andExpect(status().isOk()).andExpect(view().name("videos/videoList")); }
    @Test public void testAddVideoForm() throws Exception { mockMvc.perform(get("/videos/addVideo")).andExpect(status().isOk()).andExpect(view().name("videos/addVideo")); }
    @Test public void testAddVideoSave() throws Exception { mockMvc.perform(post("/videos/addVideo")).andExpect(status().is3xxRedirection()).andExpect(view().name("redirect:/videos/videoList")); }
}
