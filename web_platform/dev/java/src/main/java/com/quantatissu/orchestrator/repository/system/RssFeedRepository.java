package com.quantatissu.orchestrator.repository.system;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.RssFeed;

public interface RssFeedRepository extends CrudRepository<RssFeed, Integer> {
}
