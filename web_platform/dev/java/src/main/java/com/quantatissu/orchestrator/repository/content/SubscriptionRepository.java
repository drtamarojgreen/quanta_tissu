package com.quantatissu.orchestrator.repository.content;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.dbo.SubscriptionDBO;

public interface SubscriptionRepository extends CrudRepository<SubscriptionDBO, Integer> {
}
