package com.quantatissu.orchestrator.repository.commerce;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.commerce.ShopItem;

public interface ShopItemRepository extends CrudRepository<ShopItem, Integer> {
}
