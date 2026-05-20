package com.quantatissu.orchestrator.repository.system;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.chat.Chat;

public interface ChatRepository extends CrudRepository<Chat, Integer> {
}
