package com.quantatissu.orchestrator.repository.socket;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.socket.SocketSubscriber;

public interface SocketSubscriberRepository extends CrudRepository<SocketSubscriber, Integer> {
}
