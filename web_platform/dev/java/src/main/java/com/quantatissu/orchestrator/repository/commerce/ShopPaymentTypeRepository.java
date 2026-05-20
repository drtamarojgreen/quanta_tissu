package com.quantatissu.orchestrator.repository.commerce;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.shop.ShopPaymentType;

public interface ShopPaymentTypeRepository extends CrudRepository<ShopPaymentType, Integer> {
}
