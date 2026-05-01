package com.quantatissu.orchestrator.repository.content;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.review.Review;

public interface ReviewRepository extends CrudRepository<Review, Integer> {
}
