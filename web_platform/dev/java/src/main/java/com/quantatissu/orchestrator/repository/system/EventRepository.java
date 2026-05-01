package com.quantatissu.orchestrator.repository.system;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.calendar.Event;

public interface EventRepository extends CrudRepository<Event, Integer> {
}
