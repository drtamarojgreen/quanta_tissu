package com.quantatissu.orchestrator.repository.system;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.BookPage;

public interface BookPageRepository extends CrudRepository<BookPage, Integer> {
}
