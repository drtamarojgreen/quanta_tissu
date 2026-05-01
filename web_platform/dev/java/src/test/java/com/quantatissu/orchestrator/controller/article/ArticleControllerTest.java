package com.quantatissu.orchestrator.controller.article;

import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.WebMvcTest;
import org.springframework.test.web.servlet.MockMvc;

import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.post;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.view;

@WebMvcTest(ArticleController.class)
public class ArticleControllerTest {

    @Autowired private MockMvc mockMvc;

    @Test public void testArticleList() throws Exception { mockMvc.perform(get("/articles/articleList")).andExpect(status().isOk()).andExpect(view().name("articles/articleList")); }
    @Test public void testAddArticleForm() throws Exception { mockMvc.perform(get("/articles/addArticle")).andExpect(status().isOk()).andExpect(view().name("articles/addArticle")); }
    @Test public void testAddArticleSave() throws Exception { mockMvc.perform(post("/articles/addArticle")).andExpect(status().is3xxRedirection()).andExpect(view().name("redirect:/articles/articleList")); }
}
