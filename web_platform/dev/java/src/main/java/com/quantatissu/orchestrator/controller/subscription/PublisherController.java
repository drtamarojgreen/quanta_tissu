package com.quantatissu.orchestrator.controller.subscription;

import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;

@Controller
public class PublisherController {

    @RequestMapping(value = { "/publish" }, method = RequestMethod.GET)
    public String displaySocket(Model model) { return "publish"; }

    @RequestMapping(value = { "/publish" }, method = RequestMethod.POST)
    public String publishSocket(Model model) { return "redirect:/publish"; }

    @RequestMapping(value = { "/publishMessage" }, method = RequestMethod.POST)
    public String publishMessage(Model model) { return "redirect:/publish"; }

    @RequestMapping(value = { "/addMessage" }, method = RequestMethod.POST)
    public String addMessageSave(Model model) { return "redirect:/publish"; }

    @RequestMapping(value = { "/display" }, method = RequestMethod.GET)
    public String displayPublished(Model model) { return "display"; }
}
