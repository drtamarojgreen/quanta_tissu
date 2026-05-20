package com.quantatissu.orchestrator.controller.conference;

import com.quantatissu.orchestrator.model.conference.Room;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.WebMvcTest;
import org.springframework.http.MediaType;
import org.springframework.test.web.servlet.MockMvc;
import org.springframework.test.web.servlet.MvcResult;

import java.util.ArrayList;

import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.post;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.*;
import static org.junit.jupiter.api.Assertions.assertTrue;

@WebMvcTest(controllers = SocketRestController.class)
@org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc(addFilters = false)
public class SocketRestControllerTest {

    @Autowired
    private MockMvc mockMvc;

    @BeforeEach
    public void setup() {
        SocketRestController.setRooms(new ArrayList<>());
    }

    @Test
    public void testPollEvents() throws Exception {
        MvcResult result = mockMvc.perform(get("/socket")
                .accept(MediaType.TEXT_EVENT_STREAM_VALUE))
                .andExpect(request().asyncStarted())
                .andReturn();
        
        // Wait for async task to complete or timeout
        mockMvc.perform(asyncDispatch(result))
                .andExpect(status().isOk())
                .andExpect(content().contentTypeCompatibleWith("text/event-stream"));
    }

    @Test
    public void testPostForEvents_CreateRoom() throws Exception {
        MvcResult result = mockMvc.perform(post("/socket/123")
                .param("message", "testMessage")
                .param("sdp", "testSdp")
                .param("type", "offer")
                .accept(MediaType.TEXT_EVENT_STREAM_VALUE))
                .andExpect(request().asyncStarted())
                .andReturn();

        mockMvc.perform(asyncDispatch(result))
                .andExpect(status().isOk())
                .andExpect(content().string(org.hamcrest.Matchers.containsString("Post request for socketstestMessage")));
        
        // Verify room was created
        boolean found = SocketRestController.getRooms().stream().anyMatch(r -> r.getId().equals(123));
        assertTrue(found, "Room 123 should have been created in memory");
    }

    @Test
    public void testRoomCommands_CloseRoom() throws Exception {
        // Setup room
        SocketRestController.getRooms().add(new Room(999, "sdp", "offer", 1));

        MvcResult result = mockMvc.perform(post("/socket/999/close-room")
                .accept(MediaType.TEXT_EVENT_STREAM_VALUE))
                .andExpect(request().asyncStarted())
                .andReturn();

        mockMvc.perform(asyncDispatch(result))
                .andExpect(status().isOk())
                .andExpect(content().string(org.hamcrest.Matchers.containsString("Room id 999 is closed.")));
        
        // Verify room is deleted
        boolean found = SocketRestController.getRooms().stream().anyMatch(r -> r.getId().equals(999));
        assertTrue(!found, "Room 999 should have been removed");
    }

    // Helper to simulate async dispatch
    private org.springframework.test.web.servlet.RequestBuilder asyncDispatch(MvcResult result) {
        return org.springframework.test.web.servlet.request.MockMvcRequestBuilders.asyncDispatch(result);
    }
}
