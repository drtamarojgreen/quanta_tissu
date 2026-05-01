package com.quantatissu.orchestrator.controller.article;

import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;

@Controller
public class SocialController {

    @RequestMapping(value = { "/articles/socialList" }, method = RequestMethod.GET)
    public String socialList(Model model) { return "articles/socialList"; }

    @RequestMapping(value = { "/articles/addSocial" }, method = RequestMethod.GET)
    public String addSocialForm(Model model) { return "articles/addSocial"; }

    @RequestMapping(value = { "/articles/addSocial" }, method = RequestMethod.POST)
    public String addSocialSave(Model model) { return "redirect:/articles/socialList"; }

    @RequestMapping(value = { "/articles/editSocial/{id}" }, method = RequestMethod.GET)
    public String editSocialForm(Model model, @PathVariable("id") Integer id) { return "articles/editSocial"; }

    @RequestMapping(value = { "/articles/editSocial" }, method = RequestMethod.POST)
    public String editSocialSave(Model model) { return "redirect:/articles/socialList"; }
}
