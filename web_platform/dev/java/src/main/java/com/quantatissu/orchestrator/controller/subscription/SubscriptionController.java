package com.quantatissu.orchestrator.controller.subscription;

import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;

@Controller
public class SubscriptionController {

    @RequestMapping(value = { "/subscriptions/subscribers" }, method = RequestMethod.GET)
    public String userList(Model model) { return "subscriptions/subscribers"; }

    @RequestMapping(value = { "/subscriptions/subscribe" }, method = RequestMethod.GET)
    public String addSubscriberForm(Model model) { return "subscriptions/subscribe"; }

    @RequestMapping(value = { "/subscriptions/subscribe" }, method = RequestMethod.POST)
    public String addSubscriberSave(Model model) { return "redirect:/subscriptions/subscribers"; }

    @RequestMapping(value = { "/subscriptions/addSubscription" }, method = RequestMethod.GET)
    public String addSubscriptionForm(Model model) { return "subscriptions/addSubscription"; }

    @RequestMapping(value = { "/subscriptions/addSubscription" }, method = RequestMethod.POST)
    public String addSubscriptionSave(Model model) { return "redirect:/subscriptions/subscribers"; }

    @RequestMapping(value = { "/subscriptions/editSubscriber/{id}" }, method = RequestMethod.GET)
    public String editSubscriptionForm(Model model, @PathVariable("id") Integer id) { return "subscriptions/editSubscriber"; }

    @RequestMapping(value = { "/subscriptions/editSubscriber/{id}" }, method = RequestMethod.POST)
    public String editSubscriberSave(Model model, @PathVariable("id") Integer id) { return "redirect:/subscriptions/subscribers"; }

    @RequestMapping(value = { "/subscriptions/unsubscribe" }, method = RequestMethod.GET)
    public String unsubscribeForm(Model model) { return "subscriptions/unsubscribe"; }

    @RequestMapping(value = { "/subscriptions/unsubscribe" }, method = RequestMethod.POST)
    public String unsubscribeSave(Model model) { return "redirect:/subscriptions/subscribers"; }

    public void publishItem() {
        // Implementation for publishItem
    }
}
