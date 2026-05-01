package com.quantatissu.orchestrator.repository.system;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.system.DbConfig;

public interface DbConfigRepository extends CrudRepository<DbConfig, Integer> {
}
