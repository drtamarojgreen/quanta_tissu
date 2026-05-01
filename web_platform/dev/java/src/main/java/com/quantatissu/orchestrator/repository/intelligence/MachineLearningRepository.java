package com.quantatissu.orchestrator.repository.intelligence;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.aiml.MachineLearning;

public interface MachineLearningRepository extends CrudRepository<MachineLearning, Integer> {
}
