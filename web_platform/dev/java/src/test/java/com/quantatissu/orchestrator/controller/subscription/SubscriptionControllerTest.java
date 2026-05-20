package com.quantatissu.orchestrator.controller.subscription;

import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.WebMvcTest;
import org.springframework.test.web.servlet.MockMvc;

import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.post;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.view;

@WebMvcTest(controllers = SubscriptionController.class)
@org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc(addFilters = false)
public class SubscriptionControllerTest {

    @Autowired private MockMvc mockMvc;

    @Test public void testUserList() throws Exception { mockMvc.perform(get("/subscriptions/subscribers")).andExpect(status().isOk()).andExpect(view().name("subscriptions/subscribers")); }
    @Test public void testAddSubscriberForm() throws Exception { mockMvc.perform(get("/subscriptions/subscribe")).andExpect(status().isOk()).andExpect(view().name("subscriptions/subscribe")); }
    @Test public void testAddSubscriberSave() throws Exception { mockMvc.perform(post("/subscriptions/subscribe")).andExpect(status().is3xxRedirection()).andExpect(view().name("redirect:/subscriptions/subscribers")); }
}
