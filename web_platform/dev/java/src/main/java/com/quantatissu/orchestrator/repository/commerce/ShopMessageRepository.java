package com.quantatissu.orchestrator.repository.commerce;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.commerce.ShopMessage;

public interface ShopMessageRepository extends CrudRepository<ShopMessage, Integer> {
}
