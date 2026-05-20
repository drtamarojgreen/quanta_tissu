package com.quantatissu.orchestrator.repository.commerce;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.shop.ShopCart;

public interface ShopCartRepository extends CrudRepository<ShopCart, Integer> {
}
