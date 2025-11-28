cmake_minimum_required(VERSION 3.10)
project(TissLMTests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(TOKENIZER_PATH_FROM_CMAKE "Path to the tokenizer files" "")

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../quanta_tissu/tisslm/program)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../quanta_tissu/tisslm/program/core)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../quanta_tissu/tisslm/program/training)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../quanta_tissu/tisslm/program/generation)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../quanta_tissu/tisslm/program/tokenizer)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../quanta_tissu/tisslm/program/db)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../quanta_tissu/tisslm/program/retrieval)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../quanta_tissu/tisslm/program/rules)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../tissdb/json)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../tissdb/common)


add_library(tisslm_core
    ../../../quanta_tissu/tisslm/program/core/matrix.cpp
    ../../../quanta_tissu/tisslm/program/core/parameter.cpp
    ../../../quanta_tissu/tisslm/program/core/layernorm.cpp
    ../../../quanta_tissu/tisslm/program/core/dropout.cpp
    ../../../quanta_tissu/tisslm/program/core/feedforward.cpp
    ../../../quanta_tissu/tisslm/program/core/multiheadattention.cpp
    ../../../quanta_tissu/tisslm/program/core/transformerblock.cpp
    ../../../quanta_tissu/tisslm/program/core/positionalencoding.cpp
    ../../../quanta_tissu/tisslm/program/core/embedding.cpp
    ../../../quanta_tissu/tisslm/program/core/transformer_model.cpp
    ../../../quanta_tissu/tisslm/program/core/mock_embedder.cpp
    ../../../quanta_tissu/tisslm/program/db/tissdb_client.cpp
    ../../../quanta_tissu/tisslm/program/db/tissdb_lite_client.cpp
    ../../../quanta_tissu/tisslm/program/db/http_client.cpp
    ../../../tissdb/json/json.cpp
    ../../../tissdb/common/document.cpp
)

if(WIN32)
    target_link_libraries(tisslm_core ws2_32)
endif()

# Training components library
add_library(tisslm_training
    ../../../quanta_tissu/tisslm/program/training/optimizer.cpp
    ../../../quanta_tissu/tisslm/program/training/loss_function.cpp
    ../../../quanta_tissu/tisslm/program/training/trainer.cpp
    ../../../quanta_tissu/tisslm/program/training/dataset.cpp
)

# Generation components library
add_library(tisslm_generation
    ../../../quanta_tissu/tisslm/program/generation/generator.cpp
)

add_library(tisslm_tokenizer
    ../../../quanta_tissu/tisslm/program/tokenizer/tokenizer.cpp
    ../../../quanta_tissu/tisslm/program/tokenizer/pre_tokenizer.cpp
)

# Rules components library
add_library(tisslm_rules
    ../../../quanta_tissu/tisslm/program/rules/rule_enforcer.cpp
)

# Retrieval strategies library
add_library(retrieval_strategies
    ../../../quanta_tissu/tisslm/program/retrieval/retrieval_strategy.cpp
)

# Test executables
add_executable(test_db_and_retrieval
    test_db_and_retrieval.cpp
)

target_link_libraries(test_db_and_retrieval
    retrieval_strategies
    tisslm_core
)

add_executable(test_new_components
    test_new_components.cpp
)
target_link_libraries(test_new_components
    tisslm_core
)

add_executable(matrix_test
    matrix_test.cpp
)
target_link_libraries(matrix_test
    tisslm_core
)

add_executable(tokenizer_test
    tokenizer_test.cpp
)
target_link_libraries(tokenizer_test
    tisslm_tokenizer
)

add_executable(tokenizer_unicode_test
    tokenizer_unicode_test.cpp
)
target_link_libraries(tokenizer_unicode_test
    tisslm_tokenizer
)

add_executable(training_test
    training_test.cpp
)
target_link_libraries(training_test
    tisslm_core
    tisslm_training
)

add_executable(model_generation_test
    model_generation_test.cpp
)
target_link_libraries(model_generation_test
    tisslm_core
    tisslm_generation
    retrieval_strategies
)

add_executable(full_fledged_workout_cpp
    full_fledged_workout_cpp.cpp
)
target_link_libraries(full_fledged_workout_cpp
    tisslm_core
    tisslm_generation
    tisslm_tokenizer
    retrieval_strategies
)

add_executable(kv_cache_test
    kv_cache_test.cpp
)
target_link_libraries(kv_cache_test
    tisslm_core
    tisslm_generation
    tisslm_tokenizer
    retrieval_strategies
)

add_executable(rag_self_update_test
    rag_self_update_test.cpp
)
target_link_libraries(rag_self_update_test
    tisslm_core
    tisslm_generation
    tisslm_tokenizer
    retrieval_strategies
)

add_executable(experimental_sampling_test
    experimental_sampling_test.cpp
)
target_link_libraries(experimental_sampling_test
    tisslm_core
    tisslm_generation
    tisslm_tokenizer
    retrieval_strategies
)

add_executable(standard_generation_test
    standard_generation_test.cpp
)
target_link_libraries(standard_generation_test
    tisslm_core
    tisslm_generation
    tisslm_tokenizer
    retrieval_strategies
)

add_executable(rule_enforcement_test
    rule_enforcement_test.cpp
)
target_link_libraries(rule_enforcement_test
    tisslm_core
    tisslm_generation
    tisslm_tokenizer
    tisslm_rules
    retrieval_strategies
)

# Enable testing
enable_testing()

# Add tests
add_test(NAME DatabaseAndRetrievalTests 
         COMMAND test_db_and_retrieval)

add_test(NAME NewComponentsTests
         COMMAND test_new_components)

add_test(NAME MatrixTests
         COMMAND matrix_test)

add_test(NAME TokenizerTests
         COMMAND tokenizer_test)

add_test(NAME TrainingTests
         COMMAND training_test)

add_test(NAME ModelGenerationTests
         COMMAND model_generation_test)

add_test(NAME FullFledgedWorkoutCppTests
         COMMAND full_fledged_workout_cpp)

add_test(NAME KVCacheTests
         COMMAND kv_cache_test)

add_test(NAME RAGSelfUpdateTests
         COMMAND rag_self_update_test)

add_test(NAME ExperimentalSamplingTests
         COMMAND experimental_sampling_test)

add_test(NAME StandardGenerationTests
         COMMAND standard_generation_test)

add_test(NAME RuleEnforcementTests
         COMMAND rule_enforcement_test)

# Set test properties
set_tests_properties(DatabaseAndRetrievalTests PROPERTIES
    TIMEOUT 60
    LABELS "integration;database;retrieval"
)

set_tests_properties(NewComponentsTests PROPERTIES
    TIMEOUT 10
    LABELS "unit;core"
)

set_tests_properties(MatrixTests PROPERTIES
    TIMEOUT 10
    LABELS "unit;core"
)

set_tests_properties(TokenizerTests PROPERTIES
    TIMEOUT 10
    LABELS "unit;tokenizer"
)

set_tests_properties(TrainingTests PROPERTIES
    TIMEOUT 10
    LABELS "unit;training"
)

set_tests_properties(ModelGenerationTests PROPERTIES
    TIMEOUT 10
    LABELS "unit;model;generation"
)

set_tests_properties(FullFledgedWorkoutCppTests PROPERTIES
    TIMEOUT 30
    LABELS "integration;evaluation;generation"
)

set_tests_properties(KVCacheTests PROPERTIES
    TIMEOUT 60
    LABELS "integration;evaluation;kv_cache"
)

set_tests_properties(RAGSelfUpdateTests PROPERTIES
    TIMEOUT 120
    LABELS "integration;evaluation;rag;self_update"
)

set_tests_properties(ExperimentalSamplingTests PROPERTIES
    TIMEOUT 30
    LABELS "unit;evaluation;sampling"
)

set_tests_properties(StandardGenerationTests PROPERTIES
    TIMEOUT 30
    LABELS "unit;evaluation;generation"
)

set_tests_properties(RuleEnforcementTests PROPERTIES
    TIMEOUT 30
    LABELS "unit;evaluation;rules"
)

add_executable(advanced_analysis_test
    advanced_analysis_test.cpp
)
target_link_libraries(advanced_analysis_test
    tisslm_core
    tisslm_generation
    tisslm_tokenizer
    retrieval_strategies
)

add_test(NAME AdvancedAnalysisTests
         COMMAND advanced_analysis_test)

set_tests_properties(AdvancedAnalysisTests PROPERTIES
    TIMEOUT 30
    LABELS "unit;evaluation;analysis"
)

add_executable(tissdb_lite_integration_test
    tissdb_lite_integration_test.cpp
)
target_link_libraries(tissdb_lite_integration_test
    tisslm_core
)

add_test(NAME TissDBLiteIntegrationTests
         COMMAND tissdb_lite_integration_test)

set_tests_properties(TissDBLiteIntegrationTests PROPERTIES
    TIMEOUT 60
    LABELS "integration;database;lite"
)

add_executable(advanced_sampling_test
    advanced_sampling_test.cpp
)
target_link_libraries(advanced_sampling_test
    tisslm_core
    tisslm_generation
    tisslm_tokenizer
    retrieval_strategies
)

add_test(NAME AdvancedSamplingTests
         COMMAND advanced_sampling_test)

set_tests_properties(AdvancedSamplingTests PROPERTIES
    TIMEOUT 30
    LABELS "unit;evaluation;sampling;advanced"
)

add_executable(bayesian_sampling_test
    bayesian_sampling_test.cpp
)
target_link_libraries(bayesian_sampling_test
    tisslm_core
    tisslm_generation
    tisslm_tokenizer
    retrieval_strategies
)

add_test(NAME BayesianSamplingTests
         COMMAND bayesian_sampling_test)

set_tests_properties(BayesianSamplingTests PROPERTIES
    TIMEOUT 30
    LABELS "unit;evaluation;sampling;bayesian"
)

add_executable(encoding_test
    encoding_test.cpp
)
target_link_libraries(encoding_test
    tisslm_tokenizer
)

add_test(NAME EncodingTests
         COMMAND encoding_test)

set_tests_properties(EncodingTests PROPERTIES
    TIMEOUT 10
    LABELS "unit;tokenizer;encoding"
)

add_executable(checkpoint_test
    checkpoint_test.cpp
)
target_link_libraries(checkpoint_test
    tisslm_core
    tisslm_training
)

add_test(NAME CheckpointingTests
         COMMAND checkpoint_test)

set_tests_properties(CheckpointingTests PROPERTIES
    TIMEOUT 20
    LABELS "integration;training;checkpoint"
)

# Define all test executables
set(TEST_EXECUTABLES
    test_db_and_retrieval
    test_new_components
    matrix_test
    tokenizer_test
    training_test
    model_generation_test
    full_fledged_workout_cpp
    kv_cache_test
    rag_self_update_test
    experimental_sampling_test
    standard_generation_test
    rule_enforcement_test
    advanced_analysis_test
    tissdb_lite_integration_test
    advanced_sampling_test
    bayesian_sampling_test
    encoding_test
    dimension_integrity_test
    lora_test
    forward_pass_test
    backward_pass_test
    training_checkpoint_test
    transformer_block_test)

add_executable(dimension_integrity_test
    dimension_integrity_test.cpp
)
target_link_libraries(dimension_integrity_test
    tisslm_core
    tisslm_training
)

add_executable(lora_test
    lora_test.cpp
)
target_link_libraries(lora_test
    tisslm_core
)

add_executable(forward_pass_test
    forward_pass_test.cpp
)
target_link_libraries(forward_pass_test
    tisslm_core
)

add_executable(backward_pass_test
    backward_pass_test.cpp
)
target_link_libraries(backward_pass_test
    tisslm_core
)

add_executable(training_checkpoint_test
    training_checkpoint_test.cpp
)
target_link_libraries(training_checkpoint_test
    tisslm_core
    tisslm_training
)

add_executable(transformer_block_test
    transformer_block_test.cpp
)
target_link_libraries(transformer_block_test
    tisslm_core
)

# Get the project root and pass it as a compile definition to all tests
foreach(test_exe ${TEST_EXECUTABLES})
    target_compile_definitions(${test_exe} PRIVATE "TOKENIZER_PATH_FROM_CMAKE=\"${TOKENIZER_PATH_FROM_CMAKE}\"")
endforeach()

add_test(NAME LoraTests COMMAND lora_test)
add_test(NAME ForwardPassTests COMMAND forward_pass_test)
add_test(NAME BackwardPassTests COMMAND backward_pass_test)
add_test(NAME TrainingCheckpointTests COMMAND training_checkpoint_test)
add_test(NAME TransformerBlockTests
         COMMAND transformer_block_test)

set_tests_properties(LoraTests PROPERTIES
    TIMEOUT 10
    LABELS "unit;lora"
)

set_tests_properties(ForwardPassTests PROPERTIES
    TIMEOUT 10
    LABELS "unit;forward_pass"
)

set_tests_properties(BackwardPassTests PROPERTIES
    TIMEOUT 10
    LABELS "unit;backward_pass"
)

set_tests_properties(TrainingCheckpointTests PROPERTIES
    TIMEOUT 10
    LABELS "unit;training;checkpoint"
)

set_tests_properties(TransformerBlockTests PROPERTIES
    TIMEOUT 10
    LABELS "unit;transformer_block"
)

add_executable(concatenate_test concatenate_test.cpp)
target_link_libraries(concatenate_test tisslm_core)
add_test(NAME ConcatenateTests COMMAND concatenate_test)
set_tests_properties(ConcatenateTests PROPERTIES
    TIMEOUT 10
    LABELS "unit;core;concatenate"
)

add_executable(mqa_test mqa_test.cpp)
target_link_libraries(mqa_test tisslm_core)

add_executable(mha_backward_test mha_backward_test.cpp)
target_link_libraries(mha_backward_test tisslm_core)
add_test(NAME MQATests COMMAND mqa_test)
set_tests_properties(MQATests PROPERTIES
    TIMEOUT 10
    LABELS "unit;core;mqa"
)

add_executable(bug_fix_verification_test bug_fix_verification_test.cpp)
target_link_libraries(bug_fix_verification_test tisslm_core)
add_test(NAME BugFixVerificationTests COMMAND bug_fix_verification_test)

# Print build information
message(STATUS "Building TissLM Tests")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")

# --- Test Runner ---
add_executable(test_runner test_runner.cpp)
target_link_libraries(test_runner) # No extra libs needed, just standard C++

# --- Overfit Test ---
add_executable(overfit_test overfit_test.cpp)
target_link_libraries(overfit_test tisslm_core tisslm_training)
