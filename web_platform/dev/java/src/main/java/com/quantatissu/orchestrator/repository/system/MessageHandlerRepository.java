package com.quantatissu.orchestrator.repository.system;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.system.MessageHandler;

public interface MessageHandlerRepository extends CrudRepository<MessageHandler, Integer> {
}
