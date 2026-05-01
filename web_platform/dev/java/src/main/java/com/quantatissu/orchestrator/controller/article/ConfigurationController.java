package com.quantatissu.orchestrator.controller.article;

import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;

@Controller
public class ConfigurationController {

    @RequestMapping(value = { "/articles/configurationList" }, method = RequestMethod.GET)
    public String configurationList(Model model) { return "articles/configurationList"; }

    @RequestMapping(value = { "/articles/addConfiguration" }, method = RequestMethod.GET)
    public String addConfigurationForm(Model model) { return "articles/addConfiguration"; }

    @RequestMapping(value = { "/articles/addConfiguration" }, method = RequestMethod.POST)
    public String addConfigurationSave(Model model) { return "redirect:/articles/configurationList"; }

    @RequestMapping(value = { "/articles/editConfiguration/{id}" }, method = RequestMethod.GET)
    public String editConfigurationForm(Model model, @PathVariable("id") Integer id) { return "articles/editConfiguration"; }

    @RequestMapping(value = { "/articles/editConfiguration" }, method = RequestMethod.POST)
    public String editConfigurationSave(Model model) { return "redirect:/articles/configurationList"; }
}
