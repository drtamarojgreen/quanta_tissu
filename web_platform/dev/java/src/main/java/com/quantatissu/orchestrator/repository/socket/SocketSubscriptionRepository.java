package com.quantatissu.orchestrator.repository.socket;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.socket.SocketSubscription;

public interface SocketSubscriptionRepository extends CrudRepository<SocketSubscription, Integer> {
}
