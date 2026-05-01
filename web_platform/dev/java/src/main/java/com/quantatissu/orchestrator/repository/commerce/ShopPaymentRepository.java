package com.quantatissu.orchestrator.repository.commerce;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.commerce.ShopPayment;

public interface ShopPaymentRepository extends CrudRepository<ShopPayment, Integer> {
}
