package com.quantatissu.orchestrator.controller.news;

import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;

@Controller
public class InternationalNewsController {

    @RequestMapping(value = { "/international/business" }, method = RequestMethod.GET)
    public String showBusiness(Model model) { return "international/business"; }

    @RequestMapping(value = { "/international/finance" }, method = RequestMethod.GET)
    public String showFinance(Model model) { return "international/finance"; }

    @RequestMapping(value = { "/international/local" }, method = RequestMethod.GET)
    public String showLocal(Model model) { return "international/local"; }

    @RequestMapping(value = { "/international/news" }, method = RequestMethod.GET)
    public String showNews(Model model) { return "international/news"; }

    @RequestMapping(value = { "/international/politics" }, method = RequestMethod.GET)
    public String showPolitics(Model model) { return "international/politics"; }

    @RequestMapping(value = { "/international/international" }, method = RequestMethod.GET)
    public String showInternational(Model model) { return "international/international"; }
}
