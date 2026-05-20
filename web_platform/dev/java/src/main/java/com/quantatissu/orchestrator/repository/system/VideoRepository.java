package com.quantatissu.orchestrator.repository.system;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.video.Video;

public interface VideoRepository extends CrudRepository<Video, Integer> {
}
