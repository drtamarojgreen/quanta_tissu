package com.quantatissu.orchestrator.repository.game;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.game.Game;

public interface GameRepository extends CrudRepository<Game, Integer> {
}
