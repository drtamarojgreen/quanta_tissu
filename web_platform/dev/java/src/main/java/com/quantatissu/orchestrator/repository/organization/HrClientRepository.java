package com.quantatissu.orchestrator.repository.organization;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.hr.HrClient;

public interface HrClientRepository extends CrudRepository<HrClient, Integer> {
}
