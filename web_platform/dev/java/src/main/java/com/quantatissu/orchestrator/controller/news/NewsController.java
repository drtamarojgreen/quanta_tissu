package com.quantatissu.orchestrator.controller.news;

import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;

@Controller
public class NewsController {

    @RequestMapping(value = { "/news/reports" }, method = RequestMethod.GET)
    public String showReports(Model model) { return "news/reports"; }

    @RequestMapping(value = { "/news/forum" }, method = RequestMethod.GET)
    public String showForum(Model model) { return "news/forum"; }

    @RequestMapping(value = { "/news/news" }, method = RequestMethod.GET)
    public String showNews(Model model) { return "news/news"; }

    @RequestMapping(value = { "/news/freelance" }, method = RequestMethod.GET)
    public String showFreelance(Model model) { return "news/freelance"; }
}
