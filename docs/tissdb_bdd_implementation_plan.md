# TissDB BDD Test Execution Report

- **Date:** 2025-08-18 22:30:50
- **Duration:** 2.58 seconds
- **Overall Result:** FAIL

## Summary

| Metric             | Count |
| ------------------ | ----- |
| Scenarios Run      | 44      |
| Scenarios Passed   | 15    |
| Scenarios Failed   | 29    |
| Steps Run          | 115          |
| Steps Passed       | 32        |
| Steps Failed       | 29        |
| Steps Skipped      | 114        |

## Details

### Environment
- **Compilation Skipped:** No

### Errors

**Failed Steps:**

- **Step:** `Then the document with ID "prod1" in "products" should have content {"name": "Laptop", "category": "Electronics", "price": 1250}`
  - **Feature:** `update_delete_queries.feature`
  - **Scenario:** `Update a single document using a WHERE clause`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 306, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 65, in document_should_have_content
    assert response.status_code == 200
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^
AssertionError

    ```

- **Step:** `Then the document with ID "prod4" in "products" should have content {"name": "Desk Chair", "category": "Furniture", "price": 155}`
  - **Feature:** `update_delete_queries.feature`
  - **Scenario:** `Update multiple documents using a WHERE clause`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 306, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 65, in document_should_have_content
    assert response.status_code == 200
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^
AssertionError

    ```

- **Step:** `Then the document with ID "prod1" in "products" should have content {"name": "Laptop", "category": "Electronics", "price": 1200}`
  - **Feature:** `update_delete_queries.feature`
  - **Scenario:** `Update with a WHERE clause that matches no documents`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 306, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 65, in document_should_have_content
    assert response.status_code == 200
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^
AssertionError

    ```

- **Step:** `Then the document with ID "prod1" in "products" should exist`
  - **Feature:** `update_delete_queries.feature`
  - **Scenario:** `Delete with a WHERE clause that matches no documents`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 306, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_update_delete_steps.py", line 28, in document_should_exist
    assert response.status_code == 200, f"Expected document '{doc_id}' to exist, but it does not (status code: {response.status_code})."
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^
AssertionError: Expected document 'prod1' to exist, but it does not (status code: 404).

    ```

- **Step:** `Given a model and tokenizer`
  - **Feature:** `predict.feature`
  - **Scenario:** `Predicting the next token`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 306, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_kv_cache_steps.py", line 16, in context
    model = QuantaTissu(model_config)
            ^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/model.py", line 99, in __init__
    self.knowledge_base = KnowledgeBase(self.embeddings.value, tokenize, db_host=db_host, db_port=db_port)
                          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/knowledge_base.py", line 25, in __init__
    self._setup_database()
  File "/app/quanta_tissu/tisslm/knowledge_base.py", line 36, in _setup_database
    response.raise_for_status()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/models.py", line 1026, in raise_for_status
    raise HTTPError(http_error_msg, response=self)
requests.exceptions.HTTPError: 500 Server Error: Internal Server Error for url: http://127.0.0.1:8080/testdb

    ```

- **Step:** `Then the document with ID "sinew_doc" in "cpp_app_data" should have content {"source": "C++ App", "value": 42}`
  - **Feature:** `integration.feature`
  - **Scenario:** `Sinew C++ client interaction pattern with TissDB`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 306, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 65, in document_should_have_content
    assert response.status_code == 200
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^
AssertionError

    ```

- **Step:** `Then the document with ID "doc1" in "documents_collection" should not exist`
  - **Feature:** `database.feature`
  - **Scenario:** `Create, retrieve, update, and delete a document`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 787, in urlopen
    response = self._make_request(
               ^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 534, in _make_request
    response = conn.getresponse()
               ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 565, in getresponse
    httplib_response = super().getresponse()
                       ^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1430, in getresponse
    response.begin()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 331, in begin
    version, status, reason = self._read_status()
                              ^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 300, in _read_status
    raise RemoteDisconnected("Remote end closed connection without"
http.client.RemoteDisconnected: Remote end closed connection without response

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 644, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 474, in increment
    raise reraise(type(error), error, _stacktrace)
          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/util.py", line 38, in reraise
    raise value.with_traceback(tb)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 787, in urlopen
    response = self._make_request(
               ^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 534, in _make_request
    response = conn.getresponse()
               ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 565, in getresponse
    httplib_response = super().getresponse()
                       ^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1430, in getresponse
    response.begin()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 331, in begin
    version, status, reason = self._read_status()
                              ^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 300, in _read_status
    raise RemoteDisconnected("Remote end closed connection without"
urllib3.exceptions.ProtocolError: ('Connection aborted.', RemoteDisconnected('Remote end closed connection without response'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 306, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 87, in document_should_not_exist
    response = requests.get(f"{BASE_URL}/{context['db_name']}/{collection_name}/{doc_id}")
               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 73, in get
    return request("get", url, params=params, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 59, in request
    return session.request(method=method, url=url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 589, in request
    resp = self.send(prep, **send_kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 703, in send
    r = adapter.send(request, **kwargs)
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 659, in send
    raise ConnectionError(err, request=request)
requests.exceptions.ConnectionError: ('Connection aborted.', RemoteDisconnected('Remote end closed connection without response'))

    ```

- **Step:** `Given a running TissDB instance`
  - **Feature:** `database.feature`
  - **Scenario:** `Execute TissQL query`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 198, in _new_conn
    sock = connection.create_connection(
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 85, in create_connection
    raise err
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 73, in create_connection
    sock.connect(sa)
ConnectionRefusedError: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 787, in urlopen
    response = self._make_request(
               ^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 493, in _make_request
    conn.request(
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 494, in request
    self.endheaders()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1333, in endheaders
    self._send_output(message_body, encode_chunked=encode_chunked)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1093, in _send_output
    self.send(msg)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1037, in send
    self.connect()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 325, in connect
    self.sock = self._new_conn()
                ^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 213, in _new_conn
    raise NewConnectionError(
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f2cf97a3740>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 644, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cf97a3740>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 306, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 13, in running_tissdb_instance
    requests.delete(f"{BASE_URL}/{context['db_name']}")
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 157, in delete
    return request("delete", url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 59, in request
    return session.request(method=method, url=url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 589, in request
    resp = self.send(prep, **send_kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 703, in send
    r = adapter.send(request, **kwargs)
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 677, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cf97a3740>: Failed to establish a new connection: [Errno 111] Connection refused'))

    ```

- **Step:** `Given a running TissDB instance`
  - **Feature:** `database.feature`
  - **Scenario:** `Basic transaction operations`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 198, in _new_conn
    sock = connection.create_connection(
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 85, in create_connection
    raise err
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 73, in create_connection
    sock.connect(sa)
ConnectionRefusedError: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 787, in urlopen
    response = self._make_request(
               ^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 493, in _make_request
    conn.request(
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 494, in request
    self.endheaders()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1333, in endheaders
    self._send_output(message_body, encode_chunked=encode_chunked)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1093, in _send_output
    self.send(msg)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1037, in send
    self.connect()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 325, in connect
    self.sock = self._new_conn()
                ^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 213, in _new_conn
    raise NewConnectionError(
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f2cf089c950>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 644, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cf089c950>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 306, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 13, in running_tissdb_instance
    requests.delete(f"{BASE_URL}/{context['db_name']}")
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 157, in delete
    return request("delete", url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 59, in request
    return session.request(method=method, url=url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 589, in request
    resp = self.send(prep, **send_kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 703, in send
    r = adapter.send(request, **kwargs)
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 677, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cf089c950>: Failed to establish a new connection: [Errno 111] Connection refused'))

    ```

- **Step:** `Given a running TissDB instance`
  - **Feature:** `database.feature`
  - **Scenario:** `Rollback transaction`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 198, in _new_conn
    sock = connection.create_connection(
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 85, in create_connection
    raise err
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 73, in create_connection
    sock.connect(sa)
ConnectionRefusedError: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 787, in urlopen
    response = self._make_request(
               ^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 493, in _make_request
    conn.request(
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 494, in request
    self.endheaders()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1333, in endheaders
    self._send_output(message_body, encode_chunked=encode_chunked)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1093, in _send_output
    self.send(msg)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1037, in send
    self.connect()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 325, in connect
    self.sock = self._new_conn()
                ^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 213, in _new_conn
    raise NewConnectionError(
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f2cf97a30e0>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 644, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cf97a30e0>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 306, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 13, in running_tissdb_instance
    requests.delete(f"{BASE_URL}/{context['db_name']}")
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 157, in delete
    return request("delete", url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 59, in request
    return session.request(method=method, url=url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 589, in request
    resp = self.send(prep, **send_kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 703, in send
    r = adapter.send(request, **kwargs)
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 677, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cf97a30e0>: Failed to establish a new connection: [Errno 111] Connection refused'))

    ```

- **Step:** `Given a model and tokenizer`
  - **Feature:** `generate.feature`
  - **Scenario:** `Generating text with a prompt using greedy decoding`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 198, in _new_conn
    sock = connection.create_connection(
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 85, in create_connection
    raise err
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 73, in create_connection
    sock.connect(sa)
ConnectionRefusedError: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 787, in urlopen
    response = self._make_request(
               ^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 493, in _make_request
    conn.request(
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 494, in request
    self.endheaders()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1333, in endheaders
    self._send_output(message_body, encode_chunked=encode_chunked)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1093, in _send_output
    self.send(msg)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1037, in send
    self.connect()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 325, in connect
    self.sock = self._new_conn()
                ^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 213, in _new_conn
    raise NewConnectionError(
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f2cf089d4f0>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 644, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='127.0.0.1', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cf089d4f0>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 306, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_kv_cache_steps.py", line 16, in context
    model = QuantaTissu(model_config)
            ^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/model.py", line 99, in __init__
    self.knowledge_base = KnowledgeBase(self.embeddings.value, tokenize, db_host=db_host, db_port=db_port)
                          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/knowledge_base.py", line 25, in __init__
    self._setup_database()
  File "/app/quanta_tissu/tisslm/knowledge_base.py", line 34, in _setup_database
    response = requests.put(f"{self.base_url}/{self.db_name}")
               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 130, in put
    return request("put", url, data=data, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 59, in request
    return session.request(method=method, url=url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 589, in request
    resp = self.send(prep, **send_kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 703, in send
    r = adapter.send(request, **kwargs)
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 677, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='127.0.0.1', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cf089d4f0>: Failed to establish a new connection: [Errno 111] Connection refused'))

    ```

- **Step:** `Given a model and tokenizer`
  - **Feature:** `generate.feature`
  - **Scenario:** `Generating text with a prompt using sampling`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 198, in _new_conn
    sock = connection.create_connection(
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 85, in create_connection
    raise err
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 73, in create_connection
    sock.connect(sa)
ConnectionRefusedError: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 787, in urlopen
    response = self._make_request(
               ^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 493, in _make_request
    conn.request(
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 494, in request
    self.endheaders()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1333, in endheaders
    self._send_output(message_body, encode_chunked=encode_chunked)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1093, in _send_output
    self.send(msg)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1037, in send
    self.connect()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 325, in connect
    self.sock = self._new_conn()
                ^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 213, in _new_conn
    raise NewConnectionError(
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f2cfd581880>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 644, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='127.0.0.1', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cfd581880>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 306, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_kv_cache_steps.py", line 16, in context
    model = QuantaTissu(model_config)
            ^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/model.py", line 99, in __init__
    self.knowledge_base = KnowledgeBase(self.embeddings.value, tokenize, db_host=db_host, db_port=db_port)
                          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/knowledge_base.py", line 25, in __init__
    self._setup_database()
  File "/app/quanta_tissu/tisslm/knowledge_base.py", line 34, in _setup_database
    response = requests.put(f"{self.base_url}/{self.db_name}")
               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 130, in put
    return request("put", url, data=data, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 59, in request
    return session.request(method=method, url=url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 589, in request
    resp = self.send(prep, **send_kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 703, in send
    r = adapter.send(request, **kwargs)
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 677, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='127.0.0.1', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cfd581880>: Failed to establish a new connection: [Errno 111] Connection refused'))

    ```

- **Step:** `Given a running TissDB instance`
  - **Feature:** `more_database_tests.feature`
  - **Scenario:** `Batch document insertion`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 198, in _new_conn
    sock = connection.create_connection(
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 85, in create_connection
    raise err
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 73, in create_connection
    sock.connect(sa)
ConnectionRefusedError: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 787, in urlopen
    response = self._make_request(
               ^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 493, in _make_request
    conn.request(
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 494, in request
    self.endheaders()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1333, in endheaders
    self._send_output(message_body, encode_chunked=encode_chunked)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1093, in _send_output
    self.send(msg)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1037, in send
    self.connect()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 325, in connect
    self.sock = self._new_conn()
                ^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 213, in _new_conn
    raise NewConnectionError(
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f2cf97a3230>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 644, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cf97a3230>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 306, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 13, in running_tissdb_instance
    requests.delete(f"{BASE_URL}/{context['db_name']}")
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 157, in delete
    return request("delete", url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 59, in request
    return session.request(method=method, url=url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 589, in request
    resp = self.send(prep, **send_kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 703, in send
    r = adapter.send(request, **kwargs)
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 677, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cf97a3230>: Failed to establish a new connection: [Errno 111] Connection refused'))

    ```

- **Step:** `Given a running TissDB instance`
  - **Feature:** `more_database_tests.feature`
  - **Scenario:** `List all documents in a collection`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 198, in _new_conn
    sock = connection.create_connection(
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 85, in create_connection
    raise err
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 73, in create_connection
    sock.connect(sa)
ConnectionRefusedError: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 787, in urlopen
    response = self._make_request(
               ^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 493, in _make_request
    conn.request(
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 494, in request
    self.endheaders()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1333, in endheaders
    self._send_output(message_body, encode_chunked=encode_chunked)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1093, in _send_output
    self.send(msg)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1037, in send
    self.connect()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 325, in connect
    self.sock = self._new_conn()
                ^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 213, in _new_conn
    raise NewConnectionError(
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f2cf089d5b0>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 644, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cf089d5b0>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 306, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 13, in running_tissdb_instance
    requests.delete(f"{BASE_URL}/{context['db_name']}")
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 157, in delete
    return request("delete", url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 59, in request
    return session.request(method=method, url=url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 589, in request
    resp = self.send(prep, **send_kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 703, in send
    r = adapter.send(request, **kwargs)
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 677, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cf089d5b0>: Failed to establish a new connection: [Errno 111] Connection refused'))

    ```

- **Step:** `Given a running TissDB instance`
  - **Feature:** `more_database_tests.feature`
  - **Scenario:** `Query with multiple predicates`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 198, in _new_conn
    sock = connection.create_connection(
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 85, in create_connection
    raise err
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 73, in create_connection
    sock.connect(sa)
ConnectionRefusedError: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 787, in urlopen
    response = self._make_request(
               ^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 493, in _make_request
    conn.request(
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 494, in request
    self.endheaders()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1333, in endheaders
    self._send_output(message_body, encode_chunked=encode_chunked)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1093, in _send_output
    self.send(msg)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1037, in send
    self.connect()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 325, in connect
    self.sock = self._new_conn()
                ^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 213, in _new_conn
    raise NewConnectionError(
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f2cf97a3a10>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 644, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cf97a3a10>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 306, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 13, in running_tissdb_instance
    requests.delete(f"{BASE_URL}/{context['db_name']}")
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 157, in delete
    return request("delete", url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 59, in request
    return session.request(method=method, url=url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 589, in request
    resp = self.send(prep, **send_kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 703, in send
    r = adapter.send(request, **kwargs)
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 677, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cf97a3a10>: Failed to establish a new connection: [Errno 111] Connection refused'))

    ```

- **Step:** `Given a running TissDB instance`
  - **Feature:** `more_database_tests.feature`
  - **Scenario:** `Attempt to create a collection that already exists`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 198, in _new_conn
    sock = connection.create_connection(
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 85, in create_connection
    raise err
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 73, in create_connection
    sock.connect(sa)
ConnectionRefusedError: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 787, in urlopen
    response = self._make_request(
               ^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 493, in _make_request
    conn.request(
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 494, in request
    self.endheaders()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1333, in endheaders
    self._send_output(message_body, encode_chunked=encode_chunked)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1093, in _send_output
    self.send(msg)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1037, in send
    self.connect()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 325, in connect
    self.sock = self._new_conn()
                ^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 213, in _new_conn
    raise NewConnectionError(
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f2cf97a1580>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 644, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cf97a1580>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 306, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 13, in running_tissdb_instance
    requests.delete(f"{BASE_URL}/{context['db_name']}")
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 157, in delete
    return request("delete", url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 59, in request
    return session.request(method=method, url=url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 589, in request
    resp = self.send(prep, **send_kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 703, in send
    r = adapter.send(request, **kwargs)
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 677, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cf97a1580>: Failed to establish a new connection: [Errno 111] Connection refused'))

    ```

- **Step:** `Given a running TissDB instance`
  - **Feature:** `more_database_tests.feature`
  - **Scenario:** `Attempt to delete a non-existent document`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 198, in _new_conn
    sock = connection.create_connection(
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 85, in create_connection
    raise err
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 73, in create_connection
    sock.connect(sa)
ConnectionRefusedError: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 787, in urlopen
    response = self._make_request(
               ^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 493, in _make_request
    conn.request(
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 494, in request
    self.endheaders()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1333, in endheaders
    self._send_output(message_body, encode_chunked=encode_chunked)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1093, in _send_output
    self.send(msg)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1037, in send
    self.connect()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 325, in connect
    self.sock = self._new_conn()
                ^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 213, in _new_conn
    raise NewConnectionError(
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f2cf089dc70>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 644, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cf089dc70>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 306, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 13, in running_tissdb_instance
    requests.delete(f"{BASE_URL}/{context['db_name']}")
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 157, in delete
    return request("delete", url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 59, in request
    return session.request(method=method, url=url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 589, in request
    resp = self.send(prep, **send_kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 703, in send
    r = adapter.send(request, **kwargs)
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 677, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cf089dc70>: Failed to establish a new connection: [Errno 111] Connection refused'))

    ```

- **Step:** `Given a model and tokenizer`
  - **Feature:** `kv_cache.feature`
  - **Scenario:** `Generating with KV cache produces the same output as generating without it`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 198, in _new_conn
    sock = connection.create_connection(
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 85, in create_connection
    raise err
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 73, in create_connection
    sock.connect(sa)
ConnectionRefusedError: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 787, in urlopen
    response = self._make_request(
               ^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 493, in _make_request
    conn.request(
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 494, in request
    self.endheaders()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1333, in endheaders
    self._send_output(message_body, encode_chunked=encode_chunked)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1093, in _send_output
    self.send(msg)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1037, in send
    self.connect()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 325, in connect
    self.sock = self._new_conn()
                ^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 213, in _new_conn
    raise NewConnectionError(
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f2cf97a3140>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 644, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='127.0.0.1', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cf97a3140>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 306, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_kv_cache_steps.py", line 16, in context
    model = QuantaTissu(model_config)
            ^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/model.py", line 99, in __init__
    self.knowledge_base = KnowledgeBase(self.embeddings.value, tokenize, db_host=db_host, db_port=db_port)
                          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/knowledge_base.py", line 25, in __init__
    self._setup_database()
  File "/app/quanta_tissu/tisslm/knowledge_base.py", line 34, in _setup_database
    response = requests.put(f"{self.base_url}/{self.db_name}")
               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 130, in put
    return request("put", url, data=data, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 59, in request
    return session.request(method=method, url=url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 589, in request
    resp = self.send(prep, **send_kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 703, in send
    r = adapter.send(request, **kwargs)
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 677, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='127.0.0.1', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cf97a3140>: Failed to establish a new connection: [Errno 111] Connection refused'))

    ```

- **Step:** `Then the query result should be empty`
  - **Feature:** `select_queries.feature`
  - **Scenario:** `Query with no results`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 306, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_extended_database_steps.py", line 10, in query_result_should_be_empty
    assert isinstance(context['query_result'], list), f"Query result is not a list: {context['query_result']}"
                      ~~~~~~~^^^^^^^^^^^^^^^^
KeyError: 'query_result'

    ```

- **Step:** `Given a running TissDB instance`
  - **Feature:** `extended_database_tests.feature`
  - **Scenario:** `Handle various data types in documents`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 198, in _new_conn
    sock = connection.create_connection(
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 85, in create_connection
    raise err
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 73, in create_connection
    sock.connect(sa)
ConnectionRefusedError: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 787, in urlopen
    response = self._make_request(
               ^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 493, in _make_request
    conn.request(
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 494, in request
    self.endheaders()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1333, in endheaders
    self._send_output(message_body, encode_chunked=encode_chunked)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1093, in _send_output
    self.send(msg)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1037, in send
    self.connect()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 325, in connect
    self.sock = self._new_conn()
                ^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 213, in _new_conn
    raise NewConnectionError(
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f2cf97a29f0>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 644, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cf97a29f0>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 306, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 13, in running_tissdb_instance
    requests.delete(f"{BASE_URL}/{context['db_name']}")
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 157, in delete
    return request("delete", url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 59, in request
    return session.request(method=method, url=url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 589, in request
    resp = self.send(prep, **send_kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 703, in send
    r = adapter.send(request, **kwargs)
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 677, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cf97a29f0>: Failed to establish a new connection: [Errno 111] Connection refused'))

    ```

- **Step:** `Given a running TissDB instance`
  - **Feature:** `extended_database_tests.feature`
  - **Scenario:** `Query with OR predicate`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 198, in _new_conn
    sock = connection.create_connection(
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 85, in create_connection
    raise err
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 73, in create_connection
    sock.connect(sa)
ConnectionRefusedError: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 787, in urlopen
    response = self._make_request(
               ^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 493, in _make_request
    conn.request(
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 494, in request
    self.endheaders()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1333, in endheaders
    self._send_output(message_body, encode_chunked=encode_chunked)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1093, in _send_output
    self.send(msg)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1037, in send
    self.connect()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 325, in connect
    self.sock = self._new_conn()
                ^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 213, in _new_conn
    raise NewConnectionError(
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f2cf974a660>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 644, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cf974a660>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 306, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 13, in running_tissdb_instance
    requests.delete(f"{BASE_URL}/{context['db_name']}")
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 157, in delete
    return request("delete", url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 59, in request
    return session.request(method=method, url=url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 589, in request
    resp = self.send(prep, **send_kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 703, in send
    r = adapter.send(request, **kwargs)
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 677, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cf974a660>: Failed to establish a new connection: [Errno 111] Connection refused'))

    ```

- **Step:** `Given a running TissDB instance`
  - **Feature:** `extended_database_tests.feature`
  - **Scenario:** `Query with no matching results`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 198, in _new_conn
    sock = connection.create_connection(
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 85, in create_connection
    raise err
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 73, in create_connection
    sock.connect(sa)
ConnectionRefusedError: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 787, in urlopen
    response = self._make_request(
               ^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 493, in _make_request
    conn.request(
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 494, in request
    self.endheaders()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1333, in endheaders
    self._send_output(message_body, encode_chunked=encode_chunked)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1093, in _send_output
    self.send(msg)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1037, in send
    self.connect()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 325, in connect
    self.sock = self._new_conn()
                ^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 213, in _new_conn
    raise NewConnectionError(
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f2cf089c350>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 644, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cf089c350>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 306, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 13, in running_tissdb_instance
    requests.delete(f"{BASE_URL}/{context['db_name']}")
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 157, in delete
    return request("delete", url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 59, in request
    return session.request(method=method, url=url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 589, in request
    resp = self.send(prep, **send_kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 703, in send
    r = adapter.send(request, **kwargs)
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 677, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cf089c350>: Failed to establish a new connection: [Errno 111] Connection refused'))

    ```

- **Step:** `Then the resulting string should be "hello world"`
  - **Feature:** `tokenizer.feature`
  - **Scenario:** `Tokenizing and detokenizing a simple string should result in the original string`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 306, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_tokenizer_steps.py", line 18, in compare_strings
    assert context['detokenized_string'] == expected_string
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
AssertionError

    ```

- **Step:** `Then the resulting string should be "hello, world! 123? <test>"`
  - **Feature:** `tokenizer.feature`
  - **Scenario:** `Tokenizing and detokenizing a string with special characters`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 306, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_tokenizer_steps.py", line 18, in compare_strings
    assert context['detokenized_string'] == expected_string
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
AssertionError

    ```

- **Step:** `Given a knowledge base with a model and tokenizer`
  - **Feature:** `knowledge_base.feature`
  - **Scenario:** `Adding and retrieving a document`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 198, in _new_conn
    sock = connection.create_connection(
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 85, in create_connection
    raise err
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 73, in create_connection
    sock.connect(sa)
ConnectionRefusedError: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 787, in urlopen
    response = self._make_request(
               ^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 493, in _make_request
    conn.request(
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 494, in request
    self.endheaders()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1333, in endheaders
    self._send_output(message_body, encode_chunked=encode_chunked)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1093, in _send_output
    self.send(msg)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1037, in send
    self.connect()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 325, in connect
    self.sock = self._new_conn()
                ^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 213, in _new_conn
    raise NewConnectionError(
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f2cf089dc70>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 644, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='127.0.0.1', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cf089dc70>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 306, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_knowledge_base_steps.py", line 11, in knowledge_base_context
    model = QuantaTissu(model_config)
            ^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/model.py", line 99, in __init__
    self.knowledge_base = KnowledgeBase(self.embeddings.value, tokenize, db_host=db_host, db_port=db_port)
                          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/knowledge_base.py", line 25, in __init__
    self._setup_database()
  File "/app/quanta_tissu/tisslm/knowledge_base.py", line 34, in _setup_database
    response = requests.put(f"{self.base_url}/{self.db_name}")
               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 130, in put
    return request("put", url, data=data, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 59, in request
    return session.request(method=method, url=url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 589, in request
    resp = self.send(prep, **send_kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 703, in send
    r = adapter.send(request, **kwargs)
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 677, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='127.0.0.1', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cf089dc70>: Failed to establish a new connection: [Errno 111] Connection refused'))

    ```

- **Step:** `Given a knowledge base with a model and tokenizer`
  - **Feature:** `knowledge_base.feature`
  - **Scenario:** `Adding feedback to a document`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 198, in _new_conn
    sock = connection.create_connection(
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 85, in create_connection
    raise err
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 73, in create_connection
    sock.connect(sa)
ConnectionRefusedError: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 787, in urlopen
    response = self._make_request(
               ^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 493, in _make_request
    conn.request(
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 494, in request
    self.endheaders()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1333, in endheaders
    self._send_output(message_body, encode_chunked=encode_chunked)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1093, in _send_output
    self.send(msg)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1037, in send
    self.connect()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 325, in connect
    self.sock = self._new_conn()
                ^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 213, in _new_conn
    raise NewConnectionError(
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f2cf97a2c90>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 644, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='127.0.0.1', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cf97a2c90>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 306, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_knowledge_base_steps.py", line 11, in knowledge_base_context
    model = QuantaTissu(model_config)
            ^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/model.py", line 99, in __init__
    self.knowledge_base = KnowledgeBase(self.embeddings.value, tokenize, db_host=db_host, db_port=db_port)
                          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/knowledge_base.py", line 25, in __init__
    self._setup_database()
  File "/app/quanta_tissu/tisslm/knowledge_base.py", line 34, in _setup_database
    response = requests.put(f"{self.base_url}/{self.db_name}")
               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 130, in put
    return request("put", url, data=data, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 59, in request
    return session.request(method=method, url=url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 589, in request
    resp = self.send(prep, **send_kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 703, in send
    r = adapter.send(request, **kwargs)
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 677, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='127.0.0.1', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cf97a2c90>: Failed to establish a new connection: [Errno 111] Connection refused'))

    ```

- **Step:** `Given a knowledge base with a model and tokenizer`
  - **Feature:** `knowledge_base.feature`
  - **Scenario:** `Self-updating from interaction with user correction`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 198, in _new_conn
    sock = connection.create_connection(
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 85, in create_connection
    raise err
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 73, in create_connection
    sock.connect(sa)
ConnectionRefusedError: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 787, in urlopen
    response = self._make_request(
               ^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 493, in _make_request
    conn.request(
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 494, in request
    self.endheaders()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1333, in endheaders
    self._send_output(message_body, encode_chunked=encode_chunked)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1093, in _send_output
    self.send(msg)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1037, in send
    self.connect()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 325, in connect
    self.sock = self._new_conn()
                ^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 213, in _new_conn
    raise NewConnectionError(
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f2cf089eb40>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 644, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='127.0.0.1', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cf089eb40>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 306, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_knowledge_base_steps.py", line 11, in knowledge_base_context
    model = QuantaTissu(model_config)
            ^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/model.py", line 99, in __init__
    self.knowledge_base = KnowledgeBase(self.embeddings.value, tokenize, db_host=db_host, db_port=db_port)
                          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/knowledge_base.py", line 25, in __init__
    self._setup_database()
  File "/app/quanta_tissu/tisslm/knowledge_base.py", line 34, in _setup_database
    response = requests.put(f"{self.base_url}/{self.db_name}")
               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 130, in put
    return request("put", url, data=data, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 59, in request
    return session.request(method=method, url=url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 589, in request
    resp = self.send(prep, **send_kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 703, in send
    r = adapter.send(request, **kwargs)
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 677, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='127.0.0.1', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cf089eb40>: Failed to establish a new connection: [Errno 111] Connection refused'))

    ```

- **Step:** `Given a knowledge base with a model and tokenizer`
  - **Feature:** `knowledge_base.feature`
  - **Scenario:** `Self-updating from interaction without user correction`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 198, in _new_conn
    sock = connection.create_connection(
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 85, in create_connection
    raise err
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 73, in create_connection
    sock.connect(sa)
ConnectionRefusedError: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 787, in urlopen
    response = self._make_request(
               ^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 493, in _make_request
    conn.request(
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 494, in request
    self.endheaders()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1333, in endheaders
    self._send_output(message_body, encode_chunked=encode_chunked)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1093, in _send_output
    self.send(msg)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1037, in send
    self.connect()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 325, in connect
    self.sock = self._new_conn()
                ^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 213, in _new_conn
    raise NewConnectionError(
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f2cf089f650>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 644, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='127.0.0.1', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cf089f650>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 306, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_knowledge_base_steps.py", line 11, in knowledge_base_context
    model = QuantaTissu(model_config)
            ^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/model.py", line 99, in __init__
    self.knowledge_base = KnowledgeBase(self.embeddings.value, tokenize, db_host=db_host, db_port=db_port)
                          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/knowledge_base.py", line 25, in __init__
    self._setup_database()
  File "/app/quanta_tissu/tisslm/knowledge_base.py", line 34, in _setup_database
    response = requests.put(f"{self.base_url}/{self.db_name}")
               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 130, in put
    return request("put", url, data=data, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 59, in request
    return session.request(method=method, url=url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 589, in request
    resp = self.send(prep, **send_kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 703, in send
    r = adapter.send(request, **kwargs)
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 677, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='127.0.0.1', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cf089f650>: Failed to establish a new connection: [Errno 111] Connection refused'))

    ```

- **Step:** `Given a knowledge base with a model and tokenizer`
  - **Feature:** `knowledge_base.feature`
  - **Scenario:** `Getting knowledge base statistics`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 198, in _new_conn
    sock = connection.create_connection(
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 85, in create_connection
    raise err
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/connection.py", line 73, in create_connection
    sock.connect(sa)
ConnectionRefusedError: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 787, in urlopen
    response = self._make_request(
               ^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 493, in _make_request
    conn.request(
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 494, in request
    self.endheaders()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1333, in endheaders
    self._send_output(message_body, encode_chunked=encode_chunked)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1093, in _send_output
    self.send(msg)
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/http/client.py", line 1037, in send
    self.connect()
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 325, in connect
    self.sock = self._new_conn()
                ^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connection.py", line 213, in _new_conn
    raise NewConnectionError(
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f2cf97a34d0>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 644, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='127.0.0.1', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cf97a34d0>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 306, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_knowledge_base_steps.py", line 11, in knowledge_base_context
    model = QuantaTissu(model_config)
            ^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/model.py", line 99, in __init__
    self.knowledge_base = KnowledgeBase(self.embeddings.value, tokenize, db_host=db_host, db_port=db_port)
                          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/knowledge_base.py", line 25, in __init__
    self._setup_database()
  File "/app/quanta_tissu/tisslm/knowledge_base.py", line 34, in _setup_database
    response = requests.put(f"{self.base_url}/{self.db_name}")
               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 130, in put
    return request("put", url, data=data, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 59, in request
    return session.request(method=method, url=url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 589, in request
    resp = self.send(prep, **send_kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 703, in send
    r = adapter.send(request, **kwargs)
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 677, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='127.0.0.1', port=8080): Max retries exceeded with url: /testdb (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f2cf97a34d0>: Failed to establish a new connection: [Errno 111] Connection refused'))

    ```
\n## Conclusion\n\nThe test run failed. See error details above.