package com.quantatissu.orchestrator.repository.commerce;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.shop.ShopOrderCart;

public interface ShopOrderCartRepository extends CrudRepository<ShopOrderCart, Integer> {
}
