package com.quantatissu.orchestrator.repository.education;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.school.Lecturer;

public interface LecturerRepository extends CrudRepository<Lecturer, Integer> {
}
