package com.quantatissu.orchestrator.repository.commerce;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.commerce.CartItem;

public interface CartItemRepository extends CrudRepository<CartItem, Integer> {
}
