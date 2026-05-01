package com.quantatissu.orchestrator.repository.content;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.content.Journal;

public interface JournalRepository extends CrudRepository<Journal, Integer> {
}
