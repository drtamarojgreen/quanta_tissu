package com.quantatissu.orchestrator.repository.system;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.system.AccountUser;

public interface AccountUserRepository extends CrudRepository<AccountUser, Integer> {
}
