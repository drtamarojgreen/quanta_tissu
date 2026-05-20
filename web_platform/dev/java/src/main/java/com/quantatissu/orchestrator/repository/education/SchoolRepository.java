package com.quantatissu.orchestrator.repository.education;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.school.School;

public interface SchoolRepository extends CrudRepository<School, Integer> {
}
