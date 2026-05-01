package com.quantatissu.orchestrator.repository.system;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.video.Artist;

public interface ArtistRepository extends CrudRepository<Artist, Integer> {
}
