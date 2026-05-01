package com.quantatissu.orchestrator.repository.organization;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.organization.Researcher;

public interface ResearcherRepository extends CrudRepository<Researcher, Integer> {
}
