package com.quantatissu.orchestrator.repository.organization;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.research.Project;

public interface ProjectRepository extends CrudRepository<Project, Integer> {
}
