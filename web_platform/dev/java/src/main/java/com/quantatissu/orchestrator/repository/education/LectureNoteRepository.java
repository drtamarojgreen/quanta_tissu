package com.quantatissu.orchestrator.repository.education;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.education.LectureNote;

public interface LectureNoteRepository extends CrudRepository<LectureNote, Integer> {
}
