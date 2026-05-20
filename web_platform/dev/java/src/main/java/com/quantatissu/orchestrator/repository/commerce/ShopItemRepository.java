package com.quantatissu.orchestrator.repository.commerce;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.shop.ShopItem;

public interface ShopItemRepository extends CrudRepository<ShopItem, Integer> {
}
