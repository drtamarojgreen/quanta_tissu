package com.quantatissu.orchestrator.repository.socket;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.socket.SocketRequest;

public interface SocketRequestRepository extends CrudRepository<SocketRequest, Integer> {
}
