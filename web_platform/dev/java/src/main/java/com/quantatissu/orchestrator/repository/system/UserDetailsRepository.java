package com.quantatissu.orchestrator.repository.system;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.system.UserDetails;

public interface UserDetailsRepository extends CrudRepository<UserDetails, Integer> {
}
