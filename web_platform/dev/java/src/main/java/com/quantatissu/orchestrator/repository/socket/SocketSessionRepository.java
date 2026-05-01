package com.quantatissu.orchestrator.repository.socket;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.socket.SocketSession;

public interface SocketSessionRepository extends CrudRepository<SocketSession, Integer> {
}
