package com.quantatissu.orchestrator.repository.content;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.content.Review;

public interface ReviewRepository extends CrudRepository<Review, Integer> {
}
