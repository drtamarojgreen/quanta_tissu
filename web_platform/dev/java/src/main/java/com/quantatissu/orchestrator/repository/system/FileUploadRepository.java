package com.quantatissu.orchestrator.repository.system;

import org.springframework.data.repository.CrudRepository;
import com.quantatissu.orchestrator.model.system.FileUpload;

public interface FileUploadRepository extends CrudRepository<FileUpload, Integer> {
}
