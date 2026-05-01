package com.quantatissu.orchestrator.repository.content;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.content.Subscription;

public interface SubscriptionRepository extends CrudRepository<Subscription, Integer> {
}
