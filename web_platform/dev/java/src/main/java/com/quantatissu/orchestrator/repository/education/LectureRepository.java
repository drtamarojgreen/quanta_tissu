package com.quantatissu.orchestrator.repository.education;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.school.Lecture;

public interface LectureRepository extends CrudRepository<Lecture, Integer> {
}
