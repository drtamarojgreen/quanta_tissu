package com.quantatissu.orchestrator.repository.commerce;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.commerce.ShopProduct;

public interface ShopProductRepository extends CrudRepository<ShopProduct, Integer> {
}
