package com.quantatissu.orchestrator.repository.calendar;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.calendar.EventAdministrator;

public interface EventAdministratorRepository extends CrudRepository<EventAdministrator, Integer> {
}
