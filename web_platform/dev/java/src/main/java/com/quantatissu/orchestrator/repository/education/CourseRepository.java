package com.quantatissu.orchestrator.repository.education;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.school.Course;

public interface CourseRepository extends CrudRepository<Course, Integer> {
}
