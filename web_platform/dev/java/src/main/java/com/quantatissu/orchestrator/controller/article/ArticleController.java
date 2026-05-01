package com.quantatissu.orchestrator.controller.article;

import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;

@Controller
public class ArticleController {

    @RequestMapping(value = { "/articles/articleList" }, method = RequestMethod.GET)
    public String articleList(Model model) { return "articles/articleList"; }

    @RequestMapping(value = { "/articles/addArticle" }, method = RequestMethod.GET)
    public String addArticleForm(Model model) { return "articles/addArticle"; }

    @RequestMapping(value = { "/articles/addArticle" }, method = RequestMethod.POST)
    public String addArticleSave(Model model) { return "redirect:/articles/articleList"; }

    @RequestMapping(value = { "/articles/editArticle/{id}" }, method = RequestMethod.GET)
    public String editArticleForm(Model model, @PathVariable("id") Integer id) { return "articles/editArticle"; }

    @RequestMapping(value = { "/articles/editArticle" }, method = RequestMethod.POST)
    public String editArticleSave(Model model) { return "redirect:/articles/articleList"; }
}
