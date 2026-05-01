package com.quantatissu.orchestrator.repository.socket;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.socket.SocketMessage;

public interface SocketMessageRepository extends CrudRepository<SocketMessage, Integer> {
}
