# TissDB Compilation Errors

This document lists the compilation errors encountered when building the `tissdb` project.

## Errors in `quanta_tissu/tissdb/main.cpp`

- `quanta_tissu/tissdb/storage/lsm_tree.h:44:5: error: 'TransactionManager' does not name a type`

## Errors in `quanta_tissu/tissdb/api/http_server.cpp`

- `quanta_tissu/tissdb/api/http_server.cpp:19:10: fatal error: sys/socket.h: No such file or directory`

## Errors in `quanta_tissu/tissdb/query/ast.h`

- `quanta_tissu/tissdb/query/ast.h:48:8: error: using typedef-name 'using TissDB::Query::Expression = class std::variant<...>' after 'struct'`
- `quanta_tissu/tissdb/query/ast.h:25:7: note: 'using TissDB::Query::Expression = class std::variant<...>' has a previous declaration here`

## Errors in `quanta_tissu/tissdb/query/executor.cpp`

- `quanta_tissdb/query/../storage/lsm_tree.h:44:5: error: 'TransactionManager' does not name a type`
- `quanta_tissu/tissdb/query/executor.cpp:125:18: error: 'function' is not a member of 'std'`
- `quanta_tissu/tissdb/query/executor.cpp:125:27: error: expected primary-expression before 'void'`
- `quanta_tissu/tissdb/query/executor.cpp:146:13: error: 'extract_conditions' was not declared in this scope`
- `quanta_tissu/tissdb/query/executor.cpp:148:61: error: 'class TissDB::Storage::LSMTree' has no member named 'has_index'`
- `quanta_tissu/tissdb/query/executor.cpp:148:84: error: 'const struct TissDB::Query::SelectStatement' has no member named 'from_collection'`
- `quanta_tissu/tissdb/query/executor.cpp:149:81: error: 'const struct TissDB::Query::SelectStatement' has no member named 'from_collection'`
- `quanta_tissu/tissdb/query/executor.cpp:160:60: error: 'const struct TissDB::Query::SelectStatement' has no member named 'from_collection'`
- `quanta_tissu/tissdb/query/executor.cpp:167:39: error: 'class TissDB::Storage::LSMTree' has no member named 'get_all'`
- `quanta_tissu/tissdb/query/executor.cpp:167:60: error: 'const struct TissDB::Query::SelectStatement' has no member named 'from_collection'`
- `quanta_tissu/tissdb/query/executor.cpp:234:36: error: no match for 'operator==' (operand types are 'const std::variant<...>' and 'const char [2]')`
- `quanta_tissu/tissdb/query/executor.cpp:242:63: error: no matching function for call to 'get_if<...>(...)'`
- `quanta_tissu/tissdb/query/executor.cpp:281:40: error: 'class TissDB::Storage::LSMTree' has no member named 'get_all'`
- `quanta_tissu/tissdb/query/executor.cpp:327:40: error: 'class TissDB::Storage::LSMTree' has no member named 'get_all'`

## Errors in `quanta_tissu/tissdb/storage/indexer.cpp`

- `quanta_tissu/tissdb/storage/indexer.cpp:58:33: error: no match for 'operator[]' (operand types are 'std::unique_ptr<...>' and 'std::string')`
- `quanta_tissu/tissdb/storage/indexer.cpp:93:28: error: 'class std::unique_ptr<...>' has no member named 'erase'`
- `quanta_tissu/tissdb/storage/indexer.cpp:103:34: error: 'const class std::unique_ptr<...>' has no member named 'find'`
- `quanta_tissu/tissdb/storage/indexer.cpp:104:34: error: 'const class std::unique_ptr<...>' has no member named 'end'`
- `quanta_tissu/tissdb/storage/indexer.cpp:105:35: error: could not convert '{<expression error>}' from '<brace-enclosed initializer list>' to 'std::vector<std::string>'`

## Errors in `quanta_tissu/tissdb/storage/lsm_tree.cpp`

- `quanta_tissu/tissdb/storage/lsm_tree.h:44:5: error: 'TransactionManager' does not name a type`
- `quanta_tissu/tissdb/storage/lsm_tree.cpp:21:5: error: 'shutdown' was not declared in this scope`
- `quanta_tissu/tissdb/storage/lsm_tree.cpp:49:6: error: no declaration matches 'void TissDB::Storage::LSMTree::put(...)'`
- `quanta_tissu/tissdb/storage/lsm_tree.cpp:61:6: error: no declaration matches 'void TissDB::Storage::LSMTree::del(...)'`
- `quanta_tissu/tissdb/storage/lsm_tree.cpp:82:58: error: cannot convert 'const std::vector<std::string>' to 'const std::string&'`
- `quanta_tissu/tissdb/storage/lsm_tree.cpp:86:12: error: 'transaction_manager_' was not declared in this scope`
- `quanta_tissu/tissdb/storage/lsm_tree.cpp:91:30: error: 'transaction_manager_' was not declared in this scope`
- `quanta_tissu/tissdb/storage/lsm_tree.cpp:93:24: error: 'OperationType' has not been declared`
- `quanta_tissu/tissdb/storage/lsm_tree.cpp:95:31: error: 'OperationType' has not been declared`
- `quanta_tissu/tissdb/storage/lsm_tree.cpp:103:5: error: 'transaction_manager_' was not declared in this scope`
- `quanta_tissu/tissdb/storage/lsm_tree.cpp:106:6: error: no declaration matches 'void TissDB::Storage::LSMTree::shutdown()'`
