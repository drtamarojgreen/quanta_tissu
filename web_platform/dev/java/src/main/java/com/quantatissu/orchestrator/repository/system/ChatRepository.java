package com.quantatissu.orchestrator.repository.system;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.system.Chat;

public interface ChatRepository extends CrudRepository<Chat, Integer> {
}
