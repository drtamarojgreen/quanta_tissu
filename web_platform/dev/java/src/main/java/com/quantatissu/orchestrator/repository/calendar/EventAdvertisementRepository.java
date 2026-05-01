package com.quantatissu.orchestrator.repository.calendar;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.calendar.EventAdvertisement;

public interface EventAdvertisementRepository extends CrudRepository<EventAdvertisement, Integer> {
}
