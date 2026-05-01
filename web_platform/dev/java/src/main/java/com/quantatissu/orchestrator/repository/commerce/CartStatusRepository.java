package com.quantatissu.orchestrator.repository.commerce;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.commerce.CartStatus;

public interface CartStatusRepository extends CrudRepository<CartStatus, Integer> {
}
