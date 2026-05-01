package com.quantatissu.orchestrator.repository.system;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.system.RssFeed;

public interface RssFeedRepository extends CrudRepository<RssFeed, Integer> {
}
