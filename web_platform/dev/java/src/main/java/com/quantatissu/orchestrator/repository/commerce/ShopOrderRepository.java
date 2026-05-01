package com.quantatissu.orchestrator.repository.commerce;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.commerce.ShopOrder;

public interface ShopOrderRepository extends CrudRepository<ShopOrder, Integer> {
}
