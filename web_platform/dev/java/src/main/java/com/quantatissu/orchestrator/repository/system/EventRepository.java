package com.quantatissu.orchestrator.repository.system;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.system.Event;

public interface EventRepository extends CrudRepository<Event, Integer> {
}
