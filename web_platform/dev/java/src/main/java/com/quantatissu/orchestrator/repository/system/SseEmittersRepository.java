package com.quantatissu.orchestrator.repository.system;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.SseEmitters;

public interface SseEmittersRepository extends CrudRepository<SseEmitters, Integer> {
}
