package com.quantatissu.orchestrator.controller.article;

import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;

@Controller
public class BlogController {

    @RequestMapping(value = { "/articles/blogList" }, method = RequestMethod.GET)
    public String blogList(Model model) { return "articles/blogList"; }

    @RequestMapping(value = { "/articles/addBlog" }, method = RequestMethod.GET)
    public String addBlogForm(Model model) { return "articles/addBlog"; }

    @RequestMapping(value = { "/articles/addBlog" }, method = RequestMethod.POST)
    public String addBlogSave(Model model) { return "redirect:/articles/blogList"; }

    @RequestMapping(value = { "/articles/editBlog/{id}" }, method = RequestMethod.GET)
    public String editBlogForm(Model model, @PathVariable("id") Integer id) { return "articles/editBlog"; }

    @RequestMapping(value = { "/articles/editBlog" }, method = RequestMethod.POST)
    public String editBlogSave(Model model) { return "redirect:/articles/blogList"; }
}
