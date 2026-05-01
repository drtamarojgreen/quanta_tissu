package com.quantatissu.orchestrator.repository.content;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.dbo.JournalDBO;

public interface JournalRepository extends CrudRepository<JournalDBO, Integer> {
}
