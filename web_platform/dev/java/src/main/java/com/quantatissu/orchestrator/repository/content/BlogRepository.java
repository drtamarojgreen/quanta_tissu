package com.quantatissu.orchestrator.repository.content;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.content.Blog;

public interface BlogRepository extends CrudRepository<Blog, Integer> {
}
