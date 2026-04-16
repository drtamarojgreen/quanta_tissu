#include "analyzer_queue.hpp"
#include "rpc_protocol.hpp"
#include "filter_manager.hpp"
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>

using namespace rma;

void test_analyzer_queue() {
    std::cout << "Testing AnalyzerQueue..." << std::endl;
    AnalyzerQueue queue(10);

    ErrorWrapper<int> err1(ErrorType::LOGIC_BUG, 1, "test.cpp", 10, "msg1");
    ErrorWrapper<double> err2(ErrorType::WARNING, 2.0, "test.cpp", 20, "msg2");

    assert(queue.enqueue(err1));
    assert(queue.enqueue(err2));
    assert(queue.size() == 2);

    ErrorNodeBase* node1 = queue.dequeue();
    assert(node1 != nullptr);
    assert(node1->get_type() == ErrorType::LOGIC_BUG);
    delete node1;

    ErrorNodeBase* node2 = queue.dequeue();
    assert(node2 != nullptr);
    assert(node2->get_type() == ErrorType::WARNING);
    delete node2;

    assert(queue.empty());
    std::cout << "AnalyzerQueue tests passed!" << std::endl;
}

void test_filter_manager() {
    std::cout << "Testing FilterManager..." << std::endl;
    FilterManager fm;

    assert(fm.shouldDisplay(ErrorType::LOGIC_BUG));

    fm.disable(ErrorType::LOGIC_BUG);
    assert(!fm.shouldDisplay(ErrorType::LOGIC_BUG));

    fm.enable(ErrorType::LOGIC_BUG);
    assert(fm.shouldDisplay(ErrorType::LOGIC_BUG));

    fm.toggle(error_type_key(ErrorType::WARNING));
    assert(!fm.shouldDisplay(ErrorType::WARNING));

    fm.enableAll();
    assert(fm.shouldDisplay(ErrorType::WARNING));

    fm.disableAll();
    assert(!fm.shouldDisplay(ErrorType::LOGIC_BUG));

    std::cout << "FilterManager tests passed!" << std::endl;
}

void test_rpc_protocol() {
    std::cout << "Testing RpcProtocol..." << std::endl;
    RpcProtocol host;
    RpcProtocol analyzer;

    uint32_t session_id = 123;
    assert(host.init_host(session_id));
    assert(analyzer.init_analyzer(session_id));

    const char* data = "hello world";
    size_t len = strlen(data) + 1;
    assert(host.send(reinterpret_cast<const uint8_t*>(data), len, 1));

    uint8_t buffer[1024];
    uint32_t type_id = 0;
    size_t rec_len = analyzer.receive(buffer, sizeof(buffer), &type_id);

    assert(rec_len == len);
    assert(type_id == 1);
    assert(strcmp(reinterpret_cast<const char*>(buffer), data) == 0);

    std::cout << "RpcProtocol tests passed!" << std::endl;
}

int main() {
    test_analyzer_queue();
    test_filter_manager();
    test_rpc_protocol();
    std::cout << "All unit tests passed!" << std::endl;
    return 0;
}
