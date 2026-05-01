package com.quantatissu.orchestrator.repository.content;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.dbo.PublisherDBO;

public interface PublisherRepository extends CrudRepository<PublisherDBO, Integer> {
}
