package com.quantatissu.orchestrator.repository.system;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.Login;

public interface LoginRepository extends CrudRepository<Login, Integer> {
}
