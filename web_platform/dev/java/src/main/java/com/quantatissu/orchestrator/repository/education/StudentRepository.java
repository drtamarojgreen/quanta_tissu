package com.quantatissu.orchestrator.repository.education;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.education.Student;

public interface StudentRepository extends CrudRepository<Student, Integer> {
}
