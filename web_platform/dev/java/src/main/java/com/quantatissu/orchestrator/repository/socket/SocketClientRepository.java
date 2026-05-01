package com.quantatissu.orchestrator.repository.socket;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.socket.SocketClient;

public interface SocketClientRepository extends CrudRepository<SocketClient, Integer> {
}
