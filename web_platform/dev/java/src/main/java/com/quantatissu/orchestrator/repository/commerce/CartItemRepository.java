package com.quantatissu.orchestrator.repository.commerce;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.shop.CartItem;

public interface CartItemRepository extends CrudRepository<CartItem, Integer> {
}
