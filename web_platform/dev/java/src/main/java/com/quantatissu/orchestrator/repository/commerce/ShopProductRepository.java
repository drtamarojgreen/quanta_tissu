package com.quantatissu.orchestrator.repository.commerce;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.shop.ShopProduct;

public interface ShopProductRepository extends CrudRepository<ShopProduct, Integer> {
}
