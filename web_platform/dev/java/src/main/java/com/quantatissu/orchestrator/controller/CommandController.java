package com.quantatissu.orchestrator.controller;

import com.quantatissu.orchestrator.dbo.CommandDBO;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;
import java.util.ArrayList;
import java.util.List;

@RestController
@RequestMapping("/api/commands")
public class CommandController {

    @GetMapping
    public List<CommandDBO> getCommands() {
        List<CommandDBO> commands = new ArrayList<>();
        commands.add(new CommandDBO(1, "BUILD_DB", "Compiles the TissDB native binary"));
        commands.add(new CommandDBO(2, "START_TRAINING", "Initiates the model training process"));
        commands.add(new CommandDBO(3, "RUN_TESTS", "Executes the automated test suite"));
        return commands;
    }
}
