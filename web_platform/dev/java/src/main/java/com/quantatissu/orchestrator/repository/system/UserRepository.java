package com.quantatissu.orchestrator.repository.system;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.account.User;

public interface UserRepository extends CrudRepository<User, Integer> {
}
