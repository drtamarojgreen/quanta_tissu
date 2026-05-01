package com.quantatissu.orchestrator.repository.system;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.system.Video;

public interface VideoRepository extends CrudRepository<Video, Integer> {
}
