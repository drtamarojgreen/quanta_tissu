package com.quantatissu.orchestrator.controller.game;

import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/api/game")
public class GameController {

    @GetMapping("/status")
    public String getStatus() {
        return "Game module is active.";
    }
}
