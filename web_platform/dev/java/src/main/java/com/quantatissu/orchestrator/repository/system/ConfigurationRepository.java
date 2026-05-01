package com.quantatissu.orchestrator.repository.system;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.article.Configuration;

public interface ConfigurationRepository extends CrudRepository<Configuration, Integer> {
}
