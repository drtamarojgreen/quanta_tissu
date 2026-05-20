package com.quantatissu.orchestrator.repository.commerce;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.shop.ShopPayment;

public interface ShopPaymentRepository extends CrudRepository<ShopPayment, Integer> {
}
