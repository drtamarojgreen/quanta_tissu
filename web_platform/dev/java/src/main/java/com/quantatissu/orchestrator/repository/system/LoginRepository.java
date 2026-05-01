package com.quantatissu.orchestrator.repository.system;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.system.Login;

public interface LoginRepository extends CrudRepository<Login, Integer> {
}
