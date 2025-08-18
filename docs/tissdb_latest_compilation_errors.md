# TissDB Compilation Errors - 2025-08-17

This document lists the compilation errors encountered while trying to build and test the TissDB project.

## Error Log

```
g++ -std=c++17 -I.. -I. -Wall -Wextra -g -c query/executor.cpp -o query/executor.o
g++ -std=c++17 -I.. -I. -Wall -Wextra -g -c storage/lsm_tree.cpp -o storage/lsm_tree.o
g++ -std=c++17 -I.. -I. -Wall -Wextra -g -I.. -c ../tests/db/test_main.cpp -o test_main.o
 storage/lsm_tree.cpp: In member function ‘virtual void TissDB::Storage::LSMTree::create_collection(const std::string&, const TissDB::Schema&)’:
storage/lsm_tree.cpp:13:80: warning: unused parameter ‘schema’ [-Wunused-parameter]
   13 | void LSMTree::create_collection(const std::string& name, const TissDB::Schema& schema) {
      |                                                          ~~~~~~~~~~~~~~~~~~~~~~^~~~~~
storage/lsm_tree.cpp: In member function ‘virtual void TissDB::Storage::LSMTree::put(const std::string&, const std::string&, const TissDB::Document&, TissDB::Transactions::TransactionID)’:
storage/lsm_tree.cpp:35:128: warning: unused parameter ‘tid’ [-Wunused-parameter]
   35 | void LSMTree::put(const std::string& collection_name, const std::string& key, const Document& doc, Transactions::TransactionID tid) {
      |                                                                                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~^~~
storage/lsm_tree.cpp: In member function ‘virtual std::optional<std::shared_ptr<TissDB::Document> > TissDB::Storage::LSMTree::get(const std::string&, const std::string&, TissDB::Transactions::TransactionID)’:
storage/lsm_tree.cpp:44:143: warning: unused parameter ‘tid’ [-Wunused-parameter]
   44 | std::optional<std::shared_ptr<Document>> LSMTree::get(const std::string& collection_name, const std::string& key, Transactions::TransactionID tid) {
      |                                                                                                                   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~^~~
storage/lsm_tree.cpp: In member function ‘virtual void TissDB::Storage::LSMTree::del(const std::string&, const std::string&, TissDB::Transactions::TransactionID)’:
storage/lsm_tree.cpp:71:107: warning: unused parameter ‘tid’ [-Wunused-parameter]
   71 | void LSMTree::del(const std::string& collection_name, const std::string& key, Transactions::TransactionID tid) {
      |                                                                               ~~~~~~~~~~~~~~~~~~~~~~~~~~~~^~~
storage/lsm_tree.cpp: In member function ‘virtual void TissDB::Storage::LSMTree::create_index(const std::string&, const std::vector<std::__cxx11::basic_string<char> >&)’:
storage/lsm_tree.cpp:106:47: warning: unused parameter ‘collection_name’ [-Wunused-parameter]
  106 | void LSMTree::create_index(const std::string& collection_name, const std::vector<std::string>& field_names) {
      |                            ~~~~~~~~~~~~~~~~~~~^~~~~~~~~~~~~~~
storage/lsm_tree.cpp:106:96: warning: unused parameter ‘field_names’ [-Wunused-parameter]
  106 | void LSMTree::create_index(const std::string& collection_name, const std::vector<std::string>& field_names) {
      |                                                                ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~^~~~~~~~~~~
storage/lsm_tree.cpp: In member function ‘virtual std::vector<std::__cxx11::basic_string<char> > TissDB::Storage::LSMTree::find_by_index(const std::string&, const std::string&, const std::string&)’:
storage/lsm_tree.cpp:111:68: warning: unused parameter ‘collection_name’ [-Wunused-parameter]
  111 | std::vector<std::string> LSMTree::find_by_index(const std::string& collection_name, const std::string& field_name, const std::string& value) {
      |                                                 ~~~~~~~~~~~~~~~~~~~^~~~~~~~~~~~~~~
storage/lsm_tree.cpp:111:104: warning: unused parameter ‘field_name’ [-Wunused-parameter]
  111 | std::vector<std::string> LSMTree::find_by_index(const std::string& collection_name, const std::string& field_name, const std::string& value) {
      |                                                                                     ~~~~~~~~~~~~~~~~~~~^~~~~~~~~~
storage/lsm_tree.cpp:111:135: warning: unused parameter ‘value’ [-Wunused-parameter]
  111 | std::vector<std::string> LSMTree::find_by_index(const std::string& collection_name, const std::string& field_name, const std::string& value) {
      |                                                                                                                    ~~~~~~~~~~~~~~~~~~~^~~~~
storage/lsm_tree.cpp: In member function ‘std::vector<std::__cxx11::basic_string<char> > TissDB::Storage::LSMTree::find_by_index(const std::string&, const std::vector<std::__cxx11::basic_string<char> >&, const std::vector<std::__cxx11::basic_string<char> >&)’:
storage/lsm_tree.cpp:116:68: warning: unused parameter ‘collection_name’ [-Wunused-parameter]
  116 | std::vector<std::string> LSMTree::find_by_index(const std::string& collection_name, const std::vector<std::string>& field_names, const std::vector<std::string>& values) {
      |                                                 ~~~~~~~~~~~~~~~~~~~^~~~~~~~~~~~~~~
storage/lsm_tree.cpp:116:117: warning: unused parameter ‘field_names’ [-Wunused-parameter]
  116 | std::vector<std::string> LSMTree::find_by_index(const std::string& collection_name, const std::vector<std::string>& field_names, const std::vector<std::string>& values) {
      |                                                                                     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~^~~~~~~~~~~
storage/lsm_tree.cpp:116:162: warning: unused parameter ‘values’ [-Wunused-parameter]
  116 | std::vector<std::string> LSMTree::find_by_index(const std::string& collection_name, const std::vector<std::string>& field_names, const std::vector<std::string>& values) {
      |                                                                                                                                  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~^~~~~~
storage/lsm_tree.cpp: In member function ‘void TissDB::Storage::LSMTree::commit_transaction(TissDB::Transactions::TransactionID)’:
storage/lsm_tree.cpp:126:62: warning: unused parameter ‘transaction_id’ [-Wunused-parameter]
  126 | void LSMTree::commit_transaction(Transactions::TransactionID transaction_id) {
      |                                  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~^~~~~~~~~~~~~~
storage/l_tree.cpp: In member function ‘void TissDB::Storage::LSMTree::rollback_transaction(TissDB::Transactions::TransactionID)’:
storage/lsm_tree.cpp:131:64: warning: unused parameter ‘transaction_id’ [-Wunused-parameter]
  131 | void LSMTree::rollback_transaction(Transactions::TransactionID transaction_id) {
      |                                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~^~~~~~~~~~~~~~
storage/lsm_tree.cpp: In member function ‘bool TissDB::Storage::LSMTree::has_index(const std::string&, const std::vector<std::__cxx11::basic_string<char> >&)’:
storage/lsm_tree.cpp:136:44: warning: unused parameter ‘collection_name’ [-Wunused-parameter]
  136 | bool LSMTree::has_index(const std::string& collection_name, const std::vector<std::string>& field_names) {
      |                         ~~~~~~~~~~~~~~~~~~~^~~~~~~~~~~~~~~
storage/lsm_tree.cpp:136:93: warning: unused parameter ‘field_names’ [-Wunused-parameter]
  136 | bool LSMTree::has_index(const std::string& collection_name, const std::vector<std::string>& field_names) {
      |                                                             ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~^~~~~~~~~~~
storage/lsm_tree.cpp: In member function ‘std::vector<std::vector<std::__cxx11::basic_string<char> > > TissDB::Storage::LSMTree::get_available_indexes(const std::string&) const’:
storage/lsm_tree.cpp:141:89: warning: unused parameter ‘collection_name’ [-Wunused-parameter]
  141 | std::vector<std::vector<std::string>> LSMTree::get_available_indexes(const std::string& collection_name) const {
      |                                                                      ~~~~~~~~~~~~~~~~~~~^~~~~~~~~~~~~~~
In file included from ../tests/db/test_main.cpp:15:
../tests/db/test_executor.cpp: In function ‘void test_ExecutorSelectAll()’:
../tests/db/test_executor.cpp:95:57: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
   95 |     TissDB::Query::QueryResult result = executor.execute(ast);
      |                                         ~~~~~~~~~~~~~~~~^~~~~
In file included from ../tests/db/../../tissdb/common/document.h:8,
                 from ../tests/db/test_document.cpp:3,
                 from ../tests/db/test_main.cpp:4:
/usr/include/c++/13/variant:1428:7: note: ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’ is implicitly deleted because the default definition would be ill-formed:
 1428 |       variant(const variant& __rhs) = default;
      |       ^~~~~~~
/usr/include/c++/13/variant:1428:7: error: use of deleted function ‘constexpr std::_Enable_copy_move<false, false, true, true, _Tag>::_Enable_copy_move(const std::_Enable_copy_move<false, false, true, true, _Tag>&) [with _Tag = std::variant<TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement>]’
In file included from /usr/include/c++/13/bits/hashtable.h:36,
                 from /usr/include/c++/13/bits/unordered_map.h:33,
                 from /usr/include/c++/13/unordered_map:41,
                 from /usr/include/c++/13/functional:63,
                 from ../tests/db/test_framework.h:7,
                 from ../tests/db/test_main.cpp:1:
/usr/include/c++/13/bits/enable_special_members.h:160:15: note: declared here
  160 |     constexpr _Enable_copy_move(_Enable_copy_move const&) noexcept  = delete;
      |               ^~~~~~~~~~~~~~~~~
In file included from ../tests/db/test_executor.cpp:3:
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_executor.cpp: In function ‘void test_ExecutorUpdateReturnValue()’:
../tests/db/test_executor.cpp:122:57: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
  122 |     TissDB::Query::QueryResult result = executor.execute(ast);
      |                                         ~~~~~~~~~~~~~~~~^~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_executor.cpp: In function ‘void test_ExecutorDeleteReturnValue()’:
../tests/db/test_executor.cpp:142:57: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
  142 |     TissDB::Query::QueryResult result = executor.execute(ast);
      |                                         ~~~~~~~~~~~~~~~~^~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_executor.cpp: In function ‘void test_ExecutorUpdateModifyValue()’:
../tests/db/test_executor.cpp:160:21: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
  160 |     executor.execute(ast);
      |     ~~~~~~~~~~~~~~~~^~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_executor.cpp: In function ‘void test_ExecutorAggregateGroupBy()’:
../tests/db/test_executor.cpp:189:57: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
  189 |     TissDB::Query::QueryResult result = executor.execute(ast);
      |                                         ~~~~~~~~~~~~~~~~^~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_executor.cpp: In function ‘void test_ExecutorAggregateNoGroupBy()’:
../tests/db/test_executor.cpp:241:57: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
  241 |     TissDB::Query::QueryResult result = executor.execute(ast);
      |                                         ~~~~~~~~~~~~~~~~^~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_executor.cpp: In function ‘void test_ExecutorAggregateCountStar()’:
../tests/db/test_executor.cpp:277:57: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
  277 |     TissDB::Query::QueryResult result = executor.execute(ast);
      |                                         ~~~~~~~~~~~~~~~~^~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_executor.cpp: In function ‘void test_ExecutorAggregateEmptyResult()’:
../tests/db/test_executor.cpp:296:57: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
  296 |     TissDB::Query::QueryResult result = executor.execute(ast);
      |                                         ~~~~~~~~~~~~~~~~^~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_executor.cpp: In function ‘void test_ExecutorDeleteAll()’:
../tests/db/test_executor.cpp:328:21: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
  328 |     executor.execute(ast);
      |     ~~~~~~~~~~~~~~~~^~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_executor.cpp: In function ‘void test_ExecutorDeleteWithWhere()’:
../tests/db/test_executor.cpp:357:21: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
  357 |     executor.execute(ast);
      |     ~~~~~~~~~~~~~~~~^~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_executor.cpp: In function ‘void test_ExecutorUpdateAddField()’:
../tests/db/test_executor.cpp:384:21: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
  384 |     executor.execute(ast);
      |     ~~~~~~~~~~~~~~~~^~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_executor.cpp: In function ‘void test_ExecutorUpdateAll()’:
../tests/db/test_executor.cpp:427:21: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
  427 |     executor.execute(ast);
      |     ~~~~~~~~~~~~~~~~^~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_executor.cpp: In function ‘void test_ExecutorUpdateWithWhere()’:
../tests/db/test_executor.cpp:473:21: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
  473 |     executor.execute(ast);
      |     ~~~~~~~~~~~~~~~~^~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_executor.cpp: In function ‘void test_ExecutorInsert()’:
../tests/db/test_executor.cpp:517:57: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
  517 |     TissDB::Query::QueryResult result = executor.execute(ast);
      |                                         ~~~~~~~~~~~~~~~~^~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_executor.cpp: In function ‘void test_ExecutorSelectWithWhere()’:
../tests/db/test_executor.cpp:573:57: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
  573 |     TissDB::Query::QueryResult result = executor.execute(ast);
      |                                         ~~~~~~~~~~~~~~~~^~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_executor.cpp: In function ‘void test_ExecutorSelectWithLike()’:
../tests/db/test_executor.cpp:596:58: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
  596 |     TissDB::Query::QueryResult result1 = executor.execute(ast1);
      |                                          ~~~~~~~~~~~~~~~~^~~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_executor.cpp:601:58: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
  601 |     TissDB::Query::QueryResult result2 = executor.execute(ast2);
      |                                          ~~~~~~~~~~~~~~~~^~~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_executor.cpp:606:58: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
  606 |     TissDB::Query::QueryResult result3 = executor.execute(ast3);
      |                                          ~~~~~~~~~~~~~~~~^~~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_executor.cpp:611:58: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
  611 |     TissDB::Query::QueryResult result4 = executor.execute(ast4);
      |                                          ~~~~~~~~~~~~~~~~^~~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_executor.cpp:617:58: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
  617 |     TissDB::Query::QueryResult result5 = executor.execute(ast5);
      |                                          ~~~~~~~~~~~~~~~~^~~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_executor.cpp:623:58: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
  623 |     TissDB::Query::QueryResult result6 = executor.execute(ast6);
      |                                          ~~~~~~~~~~~~~~~~^~~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_executor.cpp: In function ‘void test_ExecutorSelectWithIndex()’:
../tests/db/test_executor.cpp:648:57: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
  648 |     TissDB::Query::QueryResult result = executor.execute(ast);
      |                                         ~~~~~~~~~~~~~~~~^~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_executor.cpp: In function ‘void test_ExecutorSelectWithAnd()’:
../tests/db/test_executor.cpp:668:57: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
  668 |     TissDB::Query::QueryResult result = executor.execute(ast);
      |                                         ~~~~~~~~~~~~~~~~^~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_executor.cpp: In function ‘void test_ExecutorSelectWithOr()’:
../tests/db/test_executor.cpp:688:57: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
  688 |     TissDB::Query::QueryResult result = executor.execute(ast);
      |                                         ~~~~~~~~~~~~~~~~^~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_executor.cpp: In function ‘void test_ExecutorSelectOrderBy()’:
../tests/db/test_executor.cpp:716:61: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
  716 |     TissDB::Query::QueryResult result_asc = executor.execute(ast_asc);
      |                                             ~~~~~~~~~~~~~~~~^~~~~~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_executor.cpp:725:62: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
  725 |     TissDB::Query::QueryResult result_desc = executor.execute(ast_desc);
      |                                              ~~~~~~~~~~~~~~~~^~~~~~~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_executor.cpp: In function ‘void test_ExecutorSelectLimit()’:
../tests/db/test_executor.cpp:747:57: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
  747 |     TissDB::Query::QueryResult result = executor.execute(ast);
      |                                         ~~~~~~~~~~~~~~~~^~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_executor.cpp: In function ‘void test_ExecutorSelectNotEqual()’:
../tests/db/test_executor.cpp:766:57: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
  766 |     TissDB::Query::QueryResult result = executor.execute(ast);
      |                                         ~~~~~~~~~~~~~~~~^~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_executor.cpp: In function ‘void test_ExecutorSelectLessThanOrEqual()’:
../tests/db/test_executor.cpp:786:57: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
  786 |     TissDB::Query::QueryResult result = executor.execute(ast);
      |                                         ~~~~~~~~~~~~~~~~^~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_executor.cpp: In function ‘void test_ExecutorSelectGreaterThanOrEqual()’:
../tests/db/test_executor.cpp:813:57: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
  813 |     TissDB::Query::QueryResult result = executor.execute(ast);
      |                                         ~~~~~~~~~~~~~~~~^~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_executor.cpp: In function ‘void test_ExecutorSelectWithNot()’:
../tests/db/test_executor.cpp:840:57: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
  840 |     TissDB::Query::QueryResult result = executor.execute(ast);
      |                                         ~~~~~~~~~~~~~~~~^~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_executor.cpp: In function ‘void test_ExecutorAggregateMinMax()’:
../tests/db/test_executor.cpp:860:57: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
  860 |     TissDB::Query::QueryResult result = executor.execute(ast);
      |                                         ~~~~~~~~~~~~~~~~^~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_executor.cpp: In function ‘void test_ExecutorSelectSpecificFields()’:
../tests/db/test_executor.cpp:895:57: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
  895 |     TissDB::Query::QueryResult result = executor.execute(ast);
      |                                         ~~~~~~~~~~~~~~~~^~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_executor.cpp: In function ‘void test_ExecutorAggregateHaving()’:
../tests/db/test_executor.cpp:933:57: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
  933 |     TissDB::Query::QueryResult result = executor.execute(ast);
      |                                         ~~~~~~~~~~~~~~~~^~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_executor.cpp: In function ‘void test_ExecutorSelectDistinct()’:
../tests/db/test_executor.cpp:971:57: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
  971 |     TissDB::Query::QueryResult result = executor.execute(ast);
      |                                         ~~~~~~~~~~~~~~~~^~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
In file included from ../tests/db/test_main.cpp:19:
../tests/db/test_stddev.cpp: In function ‘void test_ExecutorAggregateStdDev()’:
../tests/db/test_stddev.cpp:62:57: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
   62 |     TissDB::Query::QueryResult result = executor.execute(ast);
      |                                         ~~~~~~~~~~~~~~~~^~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
In file included from ../tests/db/test_main.cpp:20:
../tests/db/test_query_executor.cpp: In function ‘void test_ExecutorUsesSingleIndex()’:
../tests/db/test_query_executor.cpp:55:58: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
   55 |     Query::QueryResult result = fixture.executor->execute(ast);
      |                                 ~~~~~~~~~~~~~~~~~~~~~~~~~^~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_query_executor.cpp: In function ‘void test_ExecutorUsesCompoundIndex()’:
../tests/db/test_query_executor.cpp:67:58: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
   67 |     Query::QueryResult result = fixture.executor->execute(ast);
      |                                 ~~~~~~~~~~~~~~~~~~~~~~~~~^~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_query_executor.cpp: In function ‘void test_ExecutorFallsBackToSingleIndex()’:
../tests/db/test_query_executor.cpp:81:58: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
   81 |     Query::QueryResult result = fixture.executor->execute(ast);
      |                                 ~~~~~~~~~~~~~~~~~~~~~~~~~^~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
../tests/db/test_query_executor.cpp: In function ‘void test_ExecutorPerformsFullScan()’:
../tests/db/test_query_executor.cpp:95:58: error: use of deleted function ‘std::variant<_Types>::variant(const std::variant<_Types>&) [with _Types = {TissDB::Query::SelectStatement, TissDB::Query::UpdateStatement, TissDB::Query::DeleteStatement, TissDB::Query::InsertStatement}]’
   95 |     Query::QueryResult result = fixture.executor->execute(ast);
      |                                 ~~~~~~~~~~~~~~~~~~~~~~~~~^~~~~
../tests/db/../../tissdb/query/executor.h:24:29: note:   initializing argument 1 of ‘TissDB::Query::QueryResult TissDB::Query::Executor::execute(TissDB::Query::AST)’
   24 |     QueryResult execute(AST ast);
      |                         ~~~~^~~
In file included from ../tests/db/../test_tissu_sinew.cpp:2,
                 from ../tests/db/test_main.cpp:23:
../quanta_tissu/tissu_sinew.h: In member function ‘virtual void tissudb::NullLogger::info(const std::string&)’:
../quanta_tissu/tissu_sinew.h:109:34: warning: unused parameter ‘message’ [-Wunused-parameter]
  109 |     void info(const std::string& message) override {}
      |               ~~~~~~~~~~~~~~~~~~~^~~~~~~
../quanta_tissu/tissu_sinew.h: In member function ‘virtual void tissudb::NullLogger::error(const std::string&)’:
../quanta_tissu/tissu_sinew.h:110:35: warning: unused parameter ‘message’ [-Wunused-parameter]
  110 |     void error(const std::string& message) override {}
      |                ~~~~~~~~~~~~~~~~~~~^~~~~~~
In file included from ../tests/db/../../tissdb/storage/bpp_tree_wrapper.h:3,
                 from ../tests/db/../../tissdb/storage/bpp_tree.h:3,
                 from ../tests/db/test_bpp_tree.cpp:4,
                 from ../tests/db/test_main.cpp:9:
../tests/db/../../tissdb/storage/BPTree.h: In instantiation of ‘void BPTree<K, V, Comp>::put(const K&, const V&) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’:
../tests/db/../../tissdb/storage/bpp_tree_wrapper.h:21:19:   required from ‘void bpp::btree<Key, Value>::insert(const Key&, const Value&) [with Key = std::__cxx11::basic_string<char>; Value = std::__cxx11::basic_string<char>]’
../tests/db/test_bpp_tree.cpp:10:17:   required from here
../tests/db/../../tissdb/storage/BPTree.h:196:29: warning: comparison of integer expressions of different signedness: ‘int’ and ‘unsigned int’ [-Wsign-compare]
  196 |         if (pos >= 0 && pos < keys.getSize() && comp(oldKey, key) < 0) {
      |                         ~~~~^~~~~~~~~~~~~~~~
../tests/db/../../tissdb/storage/BPTree.h:198:21: warning: comparison of integer expressions of different signedness: ‘int’ and ‘unsigned int’ [-Wsign-compare]
  198 |             if (pos == keys.getSize() - 1 && (tmp = tmp->parentPtr)) {
      |                 ~~~~^~~~~~~~~~~~~~~~~~~~~
../tests/db/../../tissdb/storage/BPTree.h:207:21: warning: comparison of integer expressions of different signedness: ‘int’ and ‘unsigned int’ [-Wsign-compare]
  207 |     if (result && s > order) {
      |                   ~~^~~~~~~
../tests/db/../../tissdb/storage/BPTree.h: In instantiation of ‘V* BPTree<K, V, Comp>::get(const K&) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’:
../tests/db/../../tissdb/storage/bpp_tree_wrapper.h:25:35:   required from ‘std::optional<_Up> bpp::btree<Key, Value>::find(const Key&) [with Key = std::__cxx11::basic_string<char>; Value = std::__cxx11::basic_string<char>]’
../tests/db/test_bpp_tree.cpp:14:30:   required from here
../tests/db/../../tissdb/storage/BPTree.h:278:25: warning: comparison of integer expressions of different signedness: ‘int’ and ‘unsigned int’ [-Wsign-compare]
  278 |     if (pos >= 0 && pos < curPtr->values.getSize() && comp(curPtr->keys[pos], key) == 0) {
      |                     ~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~
../tests/db/../../tissdb/storage/BPTree.h: In instantiation of ‘void BPTree<K, V, Comp>::remove(K&) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’:
../tests/db/../../tissdb/storage/bpp_tree_wrapper.h:34:22:   required from ‘void bpp::btree<Key, Value>::erase(const Key&) [with Key = std::__cxx11::basic_string<char>; Value = std::__cxx11::basic_string<char>]’
../tests/db/test_bpp_tree.cpp:40:16:   required from here
../tests/db/../../tissdb/storage/BPTree.h:249:22: warning: comparison of integer expressions of different signedness: ‘int’ and ‘unsigned int’ [-Wsign-compare]
  249 |     if (deleteKeyPos == deleteNodePtr->keys.getSize()) {
      |         ~~~~~~~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
../tests/db/../../tissdb/storage/BPTree.h: In instantiation of ‘static std::shared_ptr<BPTree<K, V, Comp> > BPTree<K, V, Comp>::deserialize(const std::string&, Comp) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >; std::string = std::__cxx11::basic_string<char>]’:
../tests/db/../../tissdb/storage/bpp_tree_wrapper.h:57:48:   required from ‘void bpp::btree<Key, Value>::load(std::istream&) [with Key = std::__cxx11::basic_string<char>; Value = std::__cxx11::basic_string<char>; std::istream = std::basic_istream<char>]’
../tests/db/test_bpp_tree.cpp:76:19:   required from here
../tests/db/../../tissdb/storage/BPTree.h:739:14: warning: comparison of unsigned expression in ‘< 0’ is always false [-Wtype-limits]
  739 |     if (size < 0) {
      |         ~~~~~^~~
In file included from ../tests/db/../../tissdb/storage/BPTree.h:12:
../tests/db/../../tissdb/storage/List.h: In instantiation of ‘List<T, Comp>::List(List<T, Comp>&) [with T = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’:
../tests/db/../../tissdb/storage/BPTree.h:181:14:   required from ‘void BPTree<K, V, Comp>::put(const K&, const V&) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’
../tests/db/../../tissdb/storage/bpp_tree_wrapper.h:21:19:   required from ‘void bpp::btree<Key, Value>::insert(const Key&, const Value&) [with Key = std::__cxx11::basic_string<char>; Value = std::__cxx11::basic_string<char>]’
../tests/db/test_bpp_tree.cpp:10:17:   required from here
../tests/db/../../tissdb/storage/List.h:46:15: warning: ‘void* memcpy(void*, const void*, size_t)’ writing to an object of type ‘class std::__cxx11::basic_string<char>’ with no trivial copy-assignment; use copy-assignment or copy-initialization instead [-Wclass-memaccess]
   46 |         memcpy(values, list.values, byteSize(size));
      |         ~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
In file included from /usr/include/c++/13/string:54,
                 from /usr/include/c++/13/bits/locale_classes.h:40,
                 from /usr/include/c++/13/bits/ios_base.h:41,
                 from /usr/include/c++/13/ios:44,
                 from /usr/include/c++/13/ostream:40,
                 from /usr/include/c++/13/iostream:41,
                 from ../tests/db/test_framework.h:4:
/usr/include/c++/13/bits/basic_string.h:87:11: note: ‘class std::__cxx11::basic_string<char>’ declared here
   87 |     class basic_string
      |           ^~~~~~~~~~~~
../tests/db/../../tissdb/storage/BPTree.h: In instantiation of ‘bool BPTree<K, V, Comp>::putToNode(Node*, const K&, const V*, Node*) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’:
../tests/db/../../tissdb/storage/BPTree.h:189:14:   required from ‘void BPTree<K, V, Comp>::put(const K&, const V&) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’
../tests/db/../../tissdb/storage/bpp_tree_wrapper.h:21:19:   required from ‘void bpp::btree<Key, Value>::insert(const Key&, const Value&) [with Key = std::__cxx11::basic_string<char>; Value = std::__cxx11::basic_string<char>]’
../tests/db/test_bpp_tree.cpp:10:17:   required from here
../tests/db/../../tissdb/storage/BPTree.h:298:28: warning: comparison of integer expressions of different signedness: ‘int’ and ‘unsigned int’ [-Wsign-compare]
  298 |     bool present = toIndex < nodePtr->keys.getSize() && comp(key, nodePtr->keys[toIndex]) == 0;
      |                    ~~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~
../tests/db/../../tissdb/storage/BPTree.h: In instantiation of ‘int BPTree<K, V, Comp>::deleteFromNode(Node*, const K&) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’:
../tests/db/../../tissdb/storage/BPTree.h:240:24:   required from ‘void BPTree<K, V, Comp>::remove(K&) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’
../tests/db/../../tissdb/storage/bpp_tree_wrapper.h:34:22:   required from ‘void bpp::btree<Key, Value>::erase(const Key&) [with Key = std::__cxx11::basic_string<char>; Value = std::__cxx11::basic_string<char>]’
../tests/db/test_bpp_tree.cpp:40:16:   required from here
../tests/db/../../tissdb/storage/BPTree.h:366:24: warning: comparison of integer expressions of different signedness: ‘int’ and ‘unsigned int’ [-Wsign-compare]
  366 |     if (pos < 0 || pos >= nodePtr->keys.getSize() || keys[pos] != key) {
      |                    ~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~
../tests/db/../../tissdb/storage/BPTree.h: In instantiation of ‘void BPTree<K, V, Comp>::updateParentKey(Node*, K&, K&, int) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’:
../tests/db/../../tissdb/storage/BPTree.h:250:9:   required from ‘void BPTree<K, V, Comp>::remove(K&) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’
../tests/db/../../tissdb/storage/bpp_tree_wrapper.h:34:22:   required from ‘void bpp::btree<Key, Value>::erase(const Key&) [with Key = std::__cxx11::basic_string<char>; Value = std::__cxx11::basic_string<char>]’
../tests/db/test_bpp_tree.cpp:40:16:   required from here
../tests/db/../../tissdb/storage/BPTree.h:499:17: warning: comparison of integer expressions of different signedness: ‘int’ and ‘unsigned int’ [-Wsign-compare]
  499 |         if (pos == tmpNodePtr->keys.getSize() - 1 && tmpNodePtr->parentPtr) {
      |             ~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
../tests/db/../../tissdb/storage/BPTree.h: In instantiation of ‘long int BPTree<K, V, Comp>::serializeNode(const Node*, FILE*) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >; FILE = FILE]’:
../tests/db/../../tissdb/storage/BPTree.h:661:9:   required from ‘void BPTree<K, V, Comp>::serialize(std::string&) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >; std::string = std::__cxx11::basic_string<char>]’
../tests/db/../../tissdb/storage/bpp_tree_wrapper.h:41:25:   required from ‘void bpp::btree<Key, Value>::dump(std::ostream&) [with Key = std::__cxx11::basic_string<char>; Value = std::__cxx11::basic_string<char>; std::ostream = std::basic_ostream<char>]’
../tests/db/test_bpp_tree.cpp:69:19:   required from here
../tests/db/../../tissdb/storage/BPTree.h:676:23: warning: comparison of integer expressions of different signedness: ‘int’ and ‘unsigned int’ [-Wsign-compare]
  676 |     for (int i = 0; i < s; ++i) {
      |                     ~~^~~
../tests/db/../../tissdb/storage/BPTree.h:682:27: warning: comparison of integer expressions of different signedness: ‘int’ and ‘unsigned int’ [-Wsign-compare]
  682 |         for (int i = 0; i < s; ++i) {
      |                         ~~^~~
../tests/db/../../tissdb/storage/BPTree.h: In instantiation of ‘BPTree<K, V, Comp>::Node* BPTree<K, V, Comp>::deserializeNode(FILE*, Node*) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >; FILE = FILE]’:
../tests/db/../../tissdb/storage/BPTree.h:746:49:   required from ‘static std::shared_ptr<BPTree<K, V, Comp> > BPTree<K, V, Comp>::deserialize(const std::string&, Comp) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >; std::string = std::__cxx11::basic_string<char>]’
../tests/db/../../tissdb/storage/bpp_tree_wrapper.h:57:48:   required from ‘void bpp::btree<Key, Value>::load(std::istream&) [with Key = std::__cxx11::basic_string<char>; Value = std::__cxx11::basic_string<char>; std::istream = std::basic_istream<char>]’
../tests/db/test_bpp_tree.cpp:76:19:   required from here
../tests/db/../../tissdb/storage/BPTree.h:775:23: warning: comparison of integer expressions of different signedness: ‘int’ and ‘unsigned int’ [-Wsign-compare]
  775 |     for (int i = 0; i < s; ++i) {
      |                     ~~^~~
../tests/db/../../tissdb/storage/BPTree.h:779:27: warning: comparison of integer expressions of different signedness: ‘int’ and ‘unsigned int’ [-Wsign-compare]
  779 |         for (int i = 0; i < s; ++i) {
      |                         ~~^~~
../tests/db/../../tissdb/storage/BPTree.h:784:27: warning: comparison of integer expressions of different signedness: ‘int’ and ‘unsigned int’ [-Wsign-compare]
  784 |         for (int i = 0; i < s; ++i) {
      |                         ~~^~~
../tests/db/../../tissdb/storage/BPTree.h: In instantiation of ‘BPTree<K, V, Comp>::Node* BPTree<K, V, Comp>::getNextSibling(Node*) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’:
../tests/db/../../tissdb/storage/BPTree.h:755:43:   required from ‘static std::shared_ptr<BPTree<K, V, Comp> > BPTree<K, V, Comp>::deserialize(const std::string&, Comp) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >; std::string = std::__cxx11::basic_string<char>]’
../tests/db/../../tissdb/storage/bpp_tree_wrapper.h:57:48:   required from ‘void bpp::btree<Key, Value>::load(std::istream&) [with Key = std::__cxx11::basic_string<char>; Value = std::__cxx11::basic_string<char>; std::istream = std::basic_istream<char>]’
../tests/db/test_bpp_tree.cpp:76:19:   required from here
../tests/db/../../tissdb/storage/BPTree.h:812:25: warning: comparison of integer expressions of different signedness: ‘int’ and ‘unsigned int’ [-Wsign-compare]
  812 |     if (pos >= 0 && pos <= node->parentPtr->keys.getSize() && comp(key, node->parentPtr->keys[pos]) == 0) {
      |                     ~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
../tests/db/../../tissdb/storage/BPTree.h:813:17: warning: comparison of integer expressions of different signedness: ‘int’ and ‘unsigned int’ [-Wsign-compare]
  813 |         if (pos == node->parentPtr->keys.getSize() - 1) {
      |             ~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
../tests/db/../../tissdb/storage/BPTree.h: In instantiation of ‘void BPTree<K, V, Comp>::recoverLinkList(bool) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’:
../tests/db/../../tissdb/storage/BPTree.h:759:29:   required from ‘static std::shared_ptr<BPTree<K, V, Comp> > BPTree<K, V, Comp>::deserialize(const std::string&, Comp) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >; std::string = std::__cxx11::basic_string<char>]’
../tests/db/../../tissdb/storage/bpp_tree_wrapper.h:57:48:   required from ‘void bpp::btree<Key, Value>::load(std::istream&) [with Key = std::__cxx11::basic_string<char>; Value = std::__cxx11::basic_string<char>; std::istream = std::basic_istream<char>]’
../tests/db/test_bpp_tree.cpp:76:19:   required from here
../tests/db/../../tissdb/storage/BPTree.h:851:24: warning: comparison of integer expressions of different signedness: ‘int’ and ‘unsigned int’ [-Wsign-compare]
  851 |     if (valid && count != size) {
      |                  ~~~~~~^~~~~~~
../tests/db/../../tissdb/storage/List.h: In instantiation of ‘void List<T, Comp>::checkRange(int) const [with T = BPTree<std::__cxx11::basic_string<char>, std::__cxx11::basic_string<char>, DefaultCompare<std::__cxx11::basic_string<char> > >::Node*; Comp = DefaultCompare<BPTree<std::__cxx11::basic_string<char>, std::__cxx11::basic_string<char>, DefaultCompare<std::__cxx11::basic_string<char> > >::Node*>]’:
../tests/db/../../tissdb/storage/List.h:267:5:   required from ‘T& List<T, Comp>::operator[](int) [with T = BPTree<std::__cxx11::basic_string<char>, std::__cxx11::basic_string<char>, DefaultCompare<std::__cxx11::basic_string<char> > >::Node*; Comp = DefaultCompare<BPTree<std::__cxx11::basic_string<char>, std::__cxx11::basic_string<char>, DefaultCompare<std::__cxx11::basic_string<char> > >::Node*>]’
../tests/db/../../tissdb/storage/BPTree.h:186:47:   required from ‘void BPTree<K, V, Comp>::put(const K&, const V&) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’
../tests/db/../../tissdb/storage/bpp_tree_wrapper.h:21:19:   required from ‘void bpp::btree<Key, Value>::insert(const Key&, const Value&) [with Key = std::__cxx11::basic_string<char>; Value = std::__cxx11::basic_string<char>]’
../tests/db/test_bpp_tree.cpp:10:17:   required from here
../tests/db/../../tissdb/storage/List.h:204:28: warning: comparison of integer expressions of different signedness: ‘int’ and ‘const unsigned int’ [-Wsign-compare]
  204 |     if (index < 0 || index >= size) {
      |                      ~~~~~~^~~~~~~
../tests/db/../../tissdb/storage/List.h: In instantiation of ‘void List<T, Comp>::insert(int, const T&) [with T = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’:
../tests/db/../../tissdb/storage/BPTree.h:308:29:   required from ‘bool BPTree<K, V, Comp>::putToNode(Node*, const K&, const V*, Node*) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’
../tests/db/../../tissdb/storage/BPTree.h:189:14:   required from ‘void BPTree<K, V, Comp>::put(const K&, const V&) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’
../tests/db/../../tissdb/storage/bpp_tree_wrapper.h:21:19:   required from ‘void bpp::btree<Key, Value>::insert(const Key&, const Value&) [with Key = std::__cxx11::basic_string<char>; Value = std::__cxx11::basic_string<char>]’
../tests/db/test_bpp_tree.cpp:10:17:   required from here
../tests/db/../../tissdb/storage/List.h:131:12: warning: ‘void* memmove(void*, const void*, size_t)’ writing to an object of type ‘class std::__cxx11::basic_string<char>’ with no trivial copy-assignment; use copy-assignment or copy-initialization instead [-Wclass-memaccess]
  131 |     memmove(values + index + 1, values + index, byteSize(size - index));
      |     ~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/usr/include/c++/13/bits/basic_string.h:87:11: note: ‘class std::__cxx11::basic_string<char>’ declared here
   87 |     class basic_string
      |           ^~~~~~~~~~~~
../tests/db/../../tissdb/storage/List.h: In instantiation of ‘void List<T, Comp>::checkRange(int) const [with T = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’:
../tests/db/../../tissdb/storage/List.h:267:5:   required from ‘T& List<T, Comp>::operator[](int) [with T = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’
../tests/db/../../tissdb/storage/BPTree.h:195:24:   required from ‘void BPTree<K, V, Comp>::put(const K&, const V&) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’
../tests/db/../../tissdb/storage/bpp_tree_wrapper.h:21:19:   required from ‘void bpp::btree<Key, Value>::insert(const Key&, const Value&) [with Key = std::__cxx11::basic_string<char>; Value = std::__cxx11::basic_string<char>]’
../tests/db/test_bpp_tree.cpp:10:17:   required from here
../tests/db/../../tissdb/storage/List.h:204:28: warning: comparison of integer expressions of different signedness: ‘int’ and ‘const unsigned int’ [-Wsign-compare]
  204 |     if (index < 0 || index >= size) {
      |                      ~~~~~~^~~~~~~
../tests/db/../../tissdb/storage/List.h: In instantiation of ‘void List<T, Comp>::removeRange(int, int) [with T = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’:
../tests/db/../../tissdb/storage/BPTree.h:338:26:   required from ‘BPTree<K, V, Comp>::Node* BPTree<K, V, Comp>::split(Node*) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’
../tests/db/../../tissdb/storage/BPTree.h:213:27:   required from ‘void BPTree<K, V, Comp>::put(const K&, const V&) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’
../tests/db/../../tissdb/storage/bpp_tree_wrapper.h:21:19:   required from ‘void bpp::btree<Key, Value>::insert(const Key&, const Value&) [with Key = std::__cxx11::basic_string<char>; Value = std::__cxx11::basic_string<char>]’
../tests/db/test_bpp_tree.cpp:10:17:   required from here
../tests/db/../../tissdb/storage/List.h:255:12: warning: ‘void* memmove(void*, const void*, size_t)’ writing to an object of type ‘class std::__cxx11::basic_string<char>’ with no trivial copy-assignment; use copy-assignment or copy-initialization instead [-Wclass-memaccess]
  255 |     memmove(values + start, values + endIndex + 1, byteSize(size - endIndex - 1));
      |     ~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/usr/include/c++/13/bits/basic_string.h:87:11: note: ‘class std::__cxx11::basic_string<char>’ declared here
   87 |     class basic_string
      |           ^~~~~~~~~~~~
../tests/db/../../tissdb/storage/List.h: In instantiation of ‘void List<T, Comp>::removeAt(int) [with T = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’:
../tests/db/../../tissdb/storage/BPTree.h:369:18:   required from ‘int BPTree<K, V, Comp>::deleteFromNode(Node*, const K&) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’
../tests/db/../../tissdb/storage/BPTree.h:240:24:   required from ‘void BPTree<K, V, Comp>::remove(K&) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’
../tests/db/../../tissdb/storage/bpp_tree_wrapper.h:34:22:   required from ‘void bpp::btree<Key, Value>::erase(const Key&) [with Key = std::__cxx11::basic_string<char>; Value = std::__cxx11::basic_string<char>]’
../tests/db/test_bpp_tree.cpp:40:16:   required from here
../tests/db/../../tissdb/storage/List.h:212:12: warning: ‘void* memmove(void*, const void*, size_t)’ writing to an object of type ‘class std::__cxx11::basic_string<char>’ with no trivial copy-assignment; use copy-assignment or copy-initialization instead [-Wclass-memaccess]
  212 |     memmove(values + index, values + index + 1, byteSize(size - index - 1));
      |     ~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/usr/include/c++/13/bits/basic_string.h:87:11: note: ‘class std::__cxx11::basic_string<char>’ declared here
   87 |     class basic_string
      |           ^~~~~~~~~~~~
../tests/db/../../tissdb/storage/List.h: In instantiation of ‘void List<T, Comp>::insert(int, const List<T, Comp>&) [with T = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’:
../tests/db/../../tissdb/storage/BPTree.h:439:29:   required from ‘BPTree<K, V, Comp>::Node* BPTree<K, V, Comp>::fixNode(Node*) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’
../tests/db/../../tissdb/storage/BPTree.h:254:26:   required from ‘void BPTree<K, V, Comp>::remove(K&) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’
../tests/db/../../tissdb/storage/bpp_tree_wrapper.h:34:22:   required from ‘void bpp::btree<Key, Value>::erase(const Key&) [with Key = std::__cxx11::basic_string<char>; Value = std::__cxx11::basic_string<char>]’
../tests/db/test_bpp_tree.cpp:40:16:   required from here
../tests/db/../../tissdb/storage/List.h:145:12: warning: ‘void* memmove(void*, const void*, size_t)’ writing to an object of type ‘class std::__cxx11::basic_string<char>’ with no trivial copy-assignment; use copy-assignment or copy-initialization instead [-Wclass-memaccess]
  145 |     memmove(values + another.size + index, values + index, byteSize(size - index));
      |     ~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/usr/include/c++/13/bits/basic_string.h:87:11: note: ‘class std::__cxx11::basic_string<char>’ declared here
   87 |     class basic_string
      |           ^~~~~~~~~~~~
../tests/db/../../tissdb/storage/List.h:146:11: warning: ‘void* memcpy(void*, const void*, size_t)’ writing to an object of type ‘class std::__cxx11::basic_string<char>’ with no trivial copy-assignment; use copy-assignment or copy-initialization instead [-Wclass-memaccess]
  146 |     memcpy(values + index, another.values, byteSize(another.size));
      |     ~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/usr/include/c++/13/bits/basic_string.h:87:11: note: ‘class std::__cxx11::basic_string<char>’ declared here
   87 |     class basic_string
      |           ^~~~~~~~~~~~
In file included from ../tests/db/../../tissdb/storage/BPTree.h:13:
../tests/db/../../tissdb/storage/Utils.h: In instantiation of ‘std::string bp_tree_utils::stringFormat(const std::string&, Args ...) [with Args = {}; std::string = std::__cxx11::basic_string<char>]’:
../tests/db/../../tissdb/storage/BPTree.h:825:38:   required from ‘BPTree<K, V, Comp>::Node* BPTree<K, V, Comp>::getNextSibling(Node*) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’
../tests/db/../../tissdb/storage/BPTree.h:755:43:   required from ‘static std::shared_ptr<BPTree<K, V, Comp> > BPTree<K, V, Comp>::deserialize(const std::string&, Comp) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >; std::string = std::__cxx11::basic_string<char>]’
../tests/db/../../tissdb/storage/bpp_tree_wrapper.h:57:48:   required from ‘void bpp::btree<Key, Value>::load(std::istream&) [with Key = std::__cxx11::basic_string<char>; Value = std::__cxx11::basic_string<char>; std::istream = std::basic_istream<char>]’
../tests/db/test_bpp_tree.cpp:76:19:   required from here
../tests/db/../../tissdb/storage/Utils.h:104:28: warning: format not a string literal and no format arguments [-Wformat-security]
  104 |         int size = snprintf(nullptr, 0, format.c_str(), args ...) + 1;
      |                    ~~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
../tests/db/../../tissdb/storage/Utils.h:106:17: warning: format not a string literal and no format arguments [-Wformat-security]
  106 |         snprintf(buf.get(), static_cast<size_t>(size), format.c_str(), args ...);
      |         ~~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
../tests/db/../../tissdb/storage/List.h: In instantiation of ‘void List<T, Comp>::expandTo(unsigned int) [with T = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’:
../tests/db/../../tissdb/storage/List.h:129:9:   required from ‘void List<T, Comp>::insert(int, const T&) [with T = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’
../tests/db/../../tissdb/storage/BPTree.h:308:29:   required from ‘bool BPTree<K, V, Comp>::putToNode(Node*, const K&, const V*, Node*) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’
../tests/db/../../tissdb/storage/BPTree.h:189:14:   required from ‘void BPTree<K, V, Comp>::put(const K&, const V&) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’
../tests/db/../../tissdb/storage/bpp_tree_wrapper.h:21:19:   required from ‘void bpp::btree<Key, Value>::insert(const Key&, const Value&) [with Key = std::__cxx11::basic_string<char>; Value = std::__cxx11::basic_string<char>]’
../tests/db/test_bpp_tree.cpp:10:17:   required from here
../tests/db/../../tissdb/storage/List.h:240:27: warning: ‘void* realloc(void*, size_t)’ moving an object of non-trivially copyable type ‘class std::__cxx11::basic_string<char>’; use ‘new’ and ‘delete’ instead [-Wclass-memaccess]
  240 |     values = (T *) realloc(values, byteSize(cap));
      |                    ~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~
/usr/include/c++/13/bits/basic_string.h:87:11: note: ‘class std::__cxx11::basic_string<char>’ declared here
   87 |     class basic_string
      |           ^~~~~~~~~~~~
../tests/db/../../tissdb/storage/List.h: In instantiation of ‘void List<T, Comp>::insert(int, const List<T, Comp>&, int, int) [with T = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’:
../tests/db/../../tissdb/storage/List.h:123:11:   required from ‘void List<T, Comp>::add(const List<T, Comp>&, int, int) [with T = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’
../tests/db/../../tissdb/storage/BPTree.h:337:17:   required from ‘BPTree<K, V, Comp>::Node* BPTree<K, V, Comp>::split(Node*) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’
../tests/db/../../tissdb/storage/BPTree.h:213:27:   required from ‘void BPTree<K, V, Comp>::put(const K&, const V&) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’
../tests/db/../../tissdb/storage/bpp_tree_wrapper.h:21:19:   required from ‘void bpp::btree<Key, Value>::insert(const Key&, const Value&) [with Key = std::__cxx11::basic_string<char>; Value = std::__cxx11::basic_string<char>]’
../tests/db/test_bpp_tree.cpp:10:17:   required from here
../tests/db/../../tissdb/storage/List.h:162:12: warning: ‘void* memmove(void*, const void*, size_t)’ writing to an object of type ‘class std::__cxx11::basic_string<char>’ with no trivial copy-assignment; use copy-assignment or copy-initialization instead [-Wclass-memaccess]
  162 |     memmove(values + count + index, values + index, byteSize(size - index));
      |     ~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/usr/include/c++/13/bits/basic_string.h:87:11: note: ‘class std::__cxx11::basic_string<char>’ declared here
   87 |     class basic_string
      |           ^~~~~~~~~~~~
../tests/db/../../tissdb/storage/List.h:163:11: warning: ‘void* memcpy(void*, const void*, size_t)’ writing to an object of type ‘class std::__cxx11::basic_string<char>’ with no trivial copy-assignment; use copy-assignment or copy-initialization instead [-Wclass-memaccess]
  163 |     memcpy(values + index, another.values, byteSize(count));
      |     ~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/usr/include/c++/13/bits/basic_string.h:87:11: note: ‘class std::__cxx11::basic_string<char>’ declared here
   87 |     class basic_string
      |           ^~~~~~~~~~~~
../tests/db/../../tissdb/storage/List.h: In instantiation of ‘void List<T, Comp>::trimToSize() [with T = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’:
../tests/db/../../tissdb/storage/List.h:316:9:   required from ‘void List<T, Comp>::intelligentTrim() [with T = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’
../tests/db/../../tissdb/storage/List.h:257:5:   required from ‘void List<T, Comp>::removeRange(int, int) [with T = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’
../tests/db/../../tissdb/storage/BPTree.h:338:26:   required from ‘BPTree<K, V, Comp>::Node* BPTree<K, V, Comp>::split(Node*) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’
../tests/db/../../tissdb/storage/BPTree.h:213:27:   required from ‘void BPTree<K, V, Comp>::put(const K&, const V&) [with K = std::__cxx11::basic_string<char>; V = std::__cxx11::basic_string<char>; Comp = DefaultCompare<std::__cxx11::basic_string<char> >]’
../tests/db/../../tissdb/storage/bpp_tree_wrapper.h:21:19:   required from ‘void bpp::btree<Key, Value>::insert(const Key&, const Value&) [with Key = std::__cxx11::basic_string<char>; Value = std::__cxx11::basic_string<char>]’
../tests/db/test_bpp_tree.cpp:10:17:   required from here
../tests/db/../../tissdb/storage/List.h:171:31: warning: ‘void* realloc(void*, size_t)’ moving an object of non-trivially copyable type ‘class std::__cxx11::basic_string<char>’; use ‘new’ and ‘delete’ instead [-Wclass-memaccess]
  171 |         values = (T *) realloc(values, byteSize(cap));
      |                        ~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~
/usr/include/c++/13/bits/basic_string.h:87:11: note: ‘class std::__cxx11::basic_string<char>’ declared here
   87 |     class basic_string
      |           ^~~~~~~~~~~~
make: *** [Makefile:58: test_main.o] Error 1
```
