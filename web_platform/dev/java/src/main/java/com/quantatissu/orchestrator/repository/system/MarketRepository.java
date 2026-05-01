package com.quantatissu.orchestrator.repository.system;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.Market;

public interface MarketRepository extends CrudRepository<Market, Integer> {
}
