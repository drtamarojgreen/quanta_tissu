package com.quantatissu.orchestrator.repository.organization;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.organization.Topic;

public interface TopicRepository extends CrudRepository<Topic, Integer> {
}
