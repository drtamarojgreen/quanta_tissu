package com.quantatissu.orchestrator.repository.system;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.DbConfig;

public interface DbConfigRepository extends CrudRepository<DbConfig, Integer> {
}
