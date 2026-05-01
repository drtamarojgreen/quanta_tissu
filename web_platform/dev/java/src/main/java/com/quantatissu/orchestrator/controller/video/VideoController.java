package com.quantatissu.orchestrator.controller.video;

import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;

@Controller
public class VideoController {

    @RequestMapping(value = { "/videos/videoList" }, method = RequestMethod.GET)
    public String videoList(Model model) { return "videos/videoList"; }

    @RequestMapping(value = { "/videos/addVideo" }, method = RequestMethod.GET)
    public String addVideoForm(Model model) { return "videos/addVideo"; }

    @RequestMapping(value = { "/videos/addVideo" }, method = RequestMethod.POST)
    public String addVideoSave(Model model) { return "redirect:/videos/videoList"; }

    @RequestMapping(value = { "/videos/addArtist" }, method = RequestMethod.GET)
    public String addArtistForm(Model model) { return "videos/addArtist"; }

    @RequestMapping(value = { "/videos/addArtist" }, method = RequestMethod.POST)
    public String addArtistSave(Model model) { return "redirect:/videos/videoList"; }
}
