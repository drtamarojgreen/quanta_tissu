package com.quantatissu.orchestrator.repository.commerce;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.shop.ShopOrderStatus;

public interface ShopOrderStatusRepository extends CrudRepository<ShopOrderStatus, Integer> {
}
