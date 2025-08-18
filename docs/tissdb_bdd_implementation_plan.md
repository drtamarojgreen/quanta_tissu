# TissDB BDD Test Execution Report

- **Date:** 2025-08-18 07:24:18
- **Duration:** 0.55 seconds
- **Overall Result:** FAIL

## Summary

| Metric             | Count |
| ------------------ | ----- |
| Scenarios Run      | 40      |
| Scenarios Passed   | 6    |
| Scenarios Failed   | 34    |
| Steps Run          | 72          |
| Steps Passed       | 8        |
| Steps Failed       | 34        |
| Steps Skipped      | 145        |

## Details

### Environment
- **Compilation Skipped:** Yes

### Errors

**Database Start Error:**

```
Database executable not found at /app/tissdb/tissdb and compilation was skipped via NO_COMPILE flag.
```

**Failed Steps:**

- **Step:** `Then the query result should be empty`
  - **Feature:** `select_queries.feature`
  - **Scenario:** `Query with no results`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 249, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_extended_database_steps.py", line 10, in query_result_should_be_empty
    assert isinstance(context['query_result'], list), f"Query result is not a list: {context['query_result']}"
                      ~~~~~~~^^^^^^^^^^^^^^^^
KeyError: 'query_result'

    ```

- **Step:** `When I execute the TissQL query "UPDATE products SET price = 1250 WHERE name = 'Laptop'" on "products"`
  - **Feature:** `update_delete_queries.feature`
  - **Scenario:** `Update a single document using a WHERE clause`
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
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f4893edabd0>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 667, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /products/_query (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4893edabd0>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 249, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 94, in execute_tissql_query
    response = requests.post(f"{BASE_URL}/{collection_name}/_query", json=data)
               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 115, in post
    return request("post", url, data=data, json=json, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 59, in request
    return session.request(method=method, url=url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 589, in request
    resp = self.send(prep, **send_kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 703, in send
    r = adapter.send(request, **kwargs)
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 700, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /products/_query (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4893edabd0>: Failed to establish a new connection: [Errno 111] Connection refused'))

    ```

- **Step:** `When I execute the TissQL query "UPDATE products SET price = price + 5 WHERE category = 'Furniture'" on "products"`
  - **Feature:** `update_delete_queries.feature`
  - **Scenario:** `Update multiple documents using a WHERE clause`
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
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f4891a14170>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 667, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /products/_query (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891a14170>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 249, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 94, in execute_tissql_query
    response = requests.post(f"{BASE_URL}/{collection_name}/_query", json=data)
               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 115, in post
    return request("post", url, data=data, json=json, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 59, in request
    return session.request(method=method, url=url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 589, in request
    resp = self.send(prep, **send_kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 703, in send
    r = adapter.send(request, **kwargs)
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 700, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /products/_query (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891a14170>: Failed to establish a new connection: [Errno 111] Connection refused'))

    ```

- **Step:** `When I execute the TissQL query "UPDATE products SET category = 'Office Supplies' WHERE price > 2000" on "products"`
  - **Feature:** `update_delete_queries.feature`
  - **Scenario:** `Update with a WHERE clause that matches no documents`
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
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f4891a14500>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 667, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /products/_query (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891a14500>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 249, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 94, in execute_tissql_query
    response = requests.post(f"{BASE_URL}/{collection_name}/_query", json=data)
               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 115, in post
    return request("post", url, data=data, json=json, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 59, in request
    return session.request(method=method, url=url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 589, in request
    resp = self.send(prep, **send_kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 703, in send
    r = adapter.send(request, **kwargs)
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 700, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /products/_query (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891a14500>: Failed to establish a new connection: [Errno 111] Connection refused'))

    ```

- **Step:** `When I execute the TissQL query "DELETE FROM products WHERE name = 'Keyboard'" on "products"`
  - **Feature:** `update_delete_queries.feature`
  - **Scenario:** `Delete a single document using a WHERE clause`
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
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f4891a14c50>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 667, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /products/_query (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891a14c50>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 249, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 94, in execute_tissql_query
    response = requests.post(f"{BASE_URL}/{collection_name}/_query", json=data)
               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 115, in post
    return request("post", url, data=data, json=json, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 59, in request
    return session.request(method=method, url=url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 589, in request
    resp = self.send(prep, **send_kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 703, in send
    r = adapter.send(request, **kwargs)
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 700, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /products/_query (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891a14c50>: Failed to establish a new connection: [Errno 111] Connection refused'))

    ```

- **Step:** `When I execute the TissQL query "DELETE FROM products WHERE category = 'Furniture'" on "products"`
  - **Feature:** `update_delete_queries.feature`
  - **Scenario:** `Delete multiple documents using a WHERE clause`
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
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f4891a15400>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 667, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /products/_query (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891a15400>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 249, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 94, in execute_tissql_query
    response = requests.post(f"{BASE_URL}/{collection_name}/_query", json=data)
               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 115, in post
    return request("post", url, data=data, json=json, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 59, in request
    return session.request(method=method, url=url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 589, in request
    resp = self.send(prep, **send_kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 703, in send
    r = adapter.send(request, **kwargs)
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 700, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /products/_query (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891a15400>: Failed to establish a new connection: [Errno 111] Connection refused'))

    ```

- **Step:** `When I execute the TissQL query "DELETE FROM products WHERE price > 2000" on "products"`
  - **Feature:** `update_delete_queries.feature`
  - **Scenario:** `Delete with a WHERE clause that matches no documents`
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
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f4891a162a0>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 667, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /products/_query (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891a162a0>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 249, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 94, in execute_tissql_query
    response = requests.post(f"{BASE_URL}/{collection_name}/_query", json=data)
               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 115, in post
    return request("post", url, data=data, json=json, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/api.py", line 59, in request
    return session.request(method=method, url=url, **kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 589, in request
    resp = self.send(prep, **send_kwargs)
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/sessions.py", line 703, in send
    r = adapter.send(request, **kwargs)
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 700, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /products/_query (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891a162a0>: Failed to establish a new connection: [Errno 111] Connection refused'))

    ```

- **Step:** `Then the resulting string should be "hello world"`
  - **Feature:** `tokenizer.feature`
  - **Scenario:** `Tokenizing and detokenizing a simple string should result in the original string`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 249, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_tokenizer_steps.py", line 18, in compare_strings
    assert context['detokenized_string'] == expected_string
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
AssertionError

    ```

- **Step:** `Then the result should be an empty list of tokens`
  - **Feature:** `tokenizer.feature`
  - **Scenario:** `Tokenizing an empty string`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 249, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_tokenizer_steps.py", line 23, in check_empty_list
    assert context['tokens'] == []
           ^^^^^^^^^^^^^^^^^^^^^^^
ValueError: The truth value of an empty array is ambiguous. Use `array.size > 0` to check that an array is not empty.

    ```

- **Step:** `Then the resulting string should be "hello, world! 123? <test>"`
  - **Feature:** `tokenizer.feature`
  - **Scenario:** `Tokenizing and detokenizing a string with special characters`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 249, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_tokenizer_steps.py", line 18, in compare_strings
    assert context['detokenized_string'] == expected_string
           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
AssertionError

    ```

- **Step:** `Given a model and tokenizer`
  - **Feature:** `generate.feature`
  - **Scenario:** `Generating text with a prompt using greedy decoding`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 249, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_kv_cache_steps.py", line 16, in context
    model = QuantaTissu(model_config)
            ^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/model.py", line 91, in __init__
    self.embeddings = Parameter(np.random.randn(vocab_size, d_model) / np.sqrt(d_model), name="embeddings")
                                ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "numpy/random/mtrand.pyx", line 1306, in numpy.random.mtrand.RandomState.randn
  File "numpy/random/mtrand.pyx", line 1466, in numpy.random.mtrand.RandomState.standard_normal
  File "numpy/random/_common.pyx", line 655, in numpy.random._common.cont
TypeError: 'NoneType' object cannot be interpreted as an integer

    ```

- **Step:** `Given a model and tokenizer`
  - **Feature:** `generate.feature`
  - **Scenario:** `Generating text with a prompt using sampling`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 249, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_kv_cache_steps.py", line 16, in context
    model = QuantaTissu(model_config)
            ^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/model.py", line 91, in __init__
    self.embeddings = Parameter(np.random.randn(vocab_size, d_model) / np.sqrt(d_model), name="embeddings")
                                ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "numpy/random/mtrand.pyx", line 1306, in numpy.random.mtrand.RandomState.randn
  File "numpy/random/mtrand.pyx", line 1466, in numpy.random.mtrand.RandomState.standard_normal
  File "numpy/random/_common.pyx", line 655, in numpy.random._common.cont
TypeError: 'NoneType' object cannot be interpreted as an integer

    ```

- **Step:** `Given a running TissDB instance`
  - **Feature:** `database.feature`
  - **Scenario:** `Create and delete a collection`
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
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f4891a33c50>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 667, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891a33c50>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/features/steps/test_database_steps.py", line 14, in running_tissdb_instance
    response = requests.get(f"{BASE_URL}/_health")
               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
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
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 700, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891a33c50>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 249, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 17, in running_tissdb_instance
    raise Exception(f"TissDB instance is not responsive: {e}")
Exception: TissDB instance is not responsive: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891a33c50>: Failed to establish a new connection: [Errno 111] Connection refused'))

    ```

- **Step:** `Given a running TissDB instance`
  - **Feature:** `database.feature`
  - **Scenario:** `Create, retrieve, update, and delete a document`
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
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f4891794350>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 667, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891794350>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/features/steps/test_database_steps.py", line 14, in running_tissdb_instance
    response = requests.get(f"{BASE_URL}/_health")
               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
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
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 700, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891794350>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 249, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 17, in running_tissdb_instance
    raise Exception(f"TissDB instance is not responsive: {e}")
Exception: TissDB instance is not responsive: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891794350>: Failed to establish a new connection: [Errno 111] Connection refused'))

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
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f4891794b30>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 667, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891794b30>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/features/steps/test_database_steps.py", line 14, in running_tissdb_instance
    response = requests.get(f"{BASE_URL}/_health")
               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
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
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 700, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891794b30>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 249, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 17, in running_tissdb_instance
    raise Exception(f"TissDB instance is not responsive: {e}")
Exception: TissDB instance is not responsive: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891794b30>: Failed to establish a new connection: [Errno 111] Connection refused'))

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
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f4891795340>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 667, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891795340>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/features/steps/test_database_steps.py", line 14, in running_tissdb_instance
    response = requests.get(f"{BASE_URL}/_health")
               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
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
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 700, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891795340>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 249, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 17, in running_tissdb_instance
    raise Exception(f"TissDB instance is not responsive: {e}")
Exception: TissDB instance is not responsive: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891795340>: Failed to establish a new connection: [Errno 111] Connection refused'))

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
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f4891a33b60>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 667, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891a33b60>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/features/steps/test_database_steps.py", line 14, in running_tissdb_instance
    response = requests.get(f"{BASE_URL}/_health")
               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
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
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 700, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891a33b60>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 249, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 17, in running_tissdb_instance
    raise Exception(f"TissDB instance is not responsive: {e}")
Exception: TissDB instance is not responsive: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891a33b60>: Failed to establish a new connection: [Errno 111] Connection refused'))

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
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f4891794b60>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 667, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891794b60>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/features/steps/test_database_steps.py", line 14, in running_tissdb_instance
    response = requests.get(f"{BASE_URL}/_health")
               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
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
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 700, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891794b60>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 249, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 17, in running_tissdb_instance
    raise Exception(f"TissDB instance is not responsive: {e}")
Exception: TissDB instance is not responsive: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891794b60>: Failed to establish a new connection: [Errno 111] Connection refused'))

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
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f4891794230>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 667, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891794230>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/features/steps/test_database_steps.py", line 14, in running_tissdb_instance
    response = requests.get(f"{BASE_URL}/_health")
               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
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
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 700, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891794230>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 249, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 17, in running_tissdb_instance
    raise Exception(f"TissDB instance is not responsive: {e}")
Exception: TissDB instance is not responsive: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891794230>: Failed to establish a new connection: [Errno 111] Connection refused'))

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
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f4891795d30>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 667, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891795d30>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/features/steps/test_database_steps.py", line 14, in running_tissdb_instance
    response = requests.get(f"{BASE_URL}/_health")
               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
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
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 700, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891795d30>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 249, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 17, in running_tissdb_instance
    raise Exception(f"TissDB instance is not responsive: {e}")
Exception: TissDB instance is not responsive: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891795d30>: Failed to establish a new connection: [Errno 111] Connection refused'))

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
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f4891796630>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 667, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891796630>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/features/steps/test_database_steps.py", line 14, in running_tissdb_instance
    response = requests.get(f"{BASE_URL}/_health")
               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
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
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 700, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891796630>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 249, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 17, in running_tissdb_instance
    raise Exception(f"TissDB instance is not responsive: {e}")
Exception: TissDB instance is not responsive: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891796630>: Failed to establish a new connection: [Errno 111] Connection refused'))

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
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f4891a15130>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 667, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891a15130>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/features/steps/test_database_steps.py", line 14, in running_tissdb_instance
    response = requests.get(f"{BASE_URL}/_health")
               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
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
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 700, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891a15130>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 249, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 17, in running_tissdb_instance
    raise Exception(f"TissDB instance is not responsive: {e}")
Exception: TissDB instance is not responsive: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891a15130>: Failed to establish a new connection: [Errno 111] Connection refused'))

    ```

- **Step:** `Given a knowledge base with a model and tokenizer`
  - **Feature:** `knowledge_base.feature`
  - **Scenario:** `Adding and retrieving a document`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 249, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_knowledge_base_steps.py", line 11, in knowledge_base_context
    model = QuantaTissu(model_config)
            ^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/model.py", line 91, in __init__
    self.embeddings = Parameter(np.random.randn(vocab_size, d_model) / np.sqrt(d_model), name="embeddings")
                                ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "numpy/random/mtrand.pyx", line 1306, in numpy.random.mtrand.RandomState.randn
  File "numpy/random/mtrand.pyx", line 1466, in numpy.random.mtrand.RandomState.standard_normal
  File "numpy/random/_common.pyx", line 655, in numpy.random._common.cont
TypeError: 'NoneType' object cannot be interpreted as an integer

    ```

- **Step:** `Given a knowledge base with a model and tokenizer`
  - **Feature:** `knowledge_base.feature`
  - **Scenario:** `Adding feedback to a document`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 249, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_knowledge_base_steps.py", line 11, in knowledge_base_context
    model = QuantaTissu(model_config)
            ^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/model.py", line 91, in __init__
    self.embeddings = Parameter(np.random.randn(vocab_size, d_model) / np.sqrt(d_model), name="embeddings")
                                ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "numpy/random/mtrand.pyx", line 1306, in numpy.random.mtrand.RandomState.randn
  File "numpy/random/mtrand.pyx", line 1466, in numpy.random.mtrand.RandomState.standard_normal
  File "numpy/random/_common.pyx", line 655, in numpy.random._common.cont
TypeError: 'NoneType' object cannot be interpreted as an integer

    ```

- **Step:** `Given a knowledge base with a model and tokenizer`
  - **Feature:** `knowledge_base.feature`
  - **Scenario:** `Self-updating from interaction with user correction`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 249, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_knowledge_base_steps.py", line 11, in knowledge_base_context
    model = QuantaTissu(model_config)
            ^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/model.py", line 91, in __init__
    self.embeddings = Parameter(np.random.randn(vocab_size, d_model) / np.sqrt(d_model), name="embeddings")
                                ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "numpy/random/mtrand.pyx", line 1306, in numpy.random.mtrand.RandomState.randn
  File "numpy/random/mtrand.pyx", line 1466, in numpy.random.mtrand.RandomState.standard_normal
  File "numpy/random/_common.pyx", line 655, in numpy.random._common.cont
TypeError: 'NoneType' object cannot be interpreted as an integer

    ```

- **Step:** `Given a knowledge base with a model and tokenizer`
  - **Feature:** `knowledge_base.feature`
  - **Scenario:** `Self-updating from interaction without user correction`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 249, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_knowledge_base_steps.py", line 11, in knowledge_base_context
    model = QuantaTissu(model_config)
            ^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/model.py", line 91, in __init__
    self.embeddings = Parameter(np.random.randn(vocab_size, d_model) / np.sqrt(d_model), name="embeddings")
                                ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "numpy/random/mtrand.pyx", line 1306, in numpy.random.mtrand.RandomState.randn
  File "numpy/random/mtrand.pyx", line 1466, in numpy.random.mtrand.RandomState.standard_normal
  File "numpy/random/_common.pyx", line 655, in numpy.random._common.cont
TypeError: 'NoneType' object cannot be interpreted as an integer

    ```

- **Step:** `Given a knowledge base with a model and tokenizer`
  - **Feature:** `knowledge_base.feature`
  - **Scenario:** `Getting knowledge base statistics`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 249, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_knowledge_base_steps.py", line 11, in knowledge_base_context
    model = QuantaTissu(model_config)
            ^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/model.py", line 91, in __init__
    self.embeddings = Parameter(np.random.randn(vocab_size, d_model) / np.sqrt(d_model), name="embeddings")
                                ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "numpy/random/mtrand.pyx", line 1306, in numpy.random.mtrand.RandomState.randn
  File "numpy/random/mtrand.pyx", line 1466, in numpy.random.mtrand.RandomState.standard_normal
  File "numpy/random/_common.pyx", line 655, in numpy.random._common.cont
TypeError: 'NoneType' object cannot be interpreted as an integer

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
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f4891a33c80>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 667, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891a33c80>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/features/steps/test_database_steps.py", line 14, in running_tissdb_instance
    response = requests.get(f"{BASE_URL}/_health")
               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
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
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 700, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891a33c80>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 249, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 17, in running_tissdb_instance
    raise Exception(f"TissDB instance is not responsive: {e}")
Exception: TissDB instance is not responsive: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891a33c80>: Failed to establish a new connection: [Errno 111] Connection refused'))

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
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f48917964e0>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 667, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f48917964e0>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/features/steps/test_database_steps.py", line 14, in running_tissdb_instance
    response = requests.get(f"{BASE_URL}/_health")
               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
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
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 700, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f48917964e0>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 249, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 17, in running_tissdb_instance
    raise Exception(f"TissDB instance is not responsive: {e}")
Exception: TissDB instance is not responsive: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f48917964e0>: Failed to establish a new connection: [Errno 111] Connection refused'))

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
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f4891795c10>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 667, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891795c10>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/features/steps/test_database_steps.py", line 14, in running_tissdb_instance
    response = requests.get(f"{BASE_URL}/_health")
               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
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
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 700, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891795c10>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 249, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 17, in running_tissdb_instance
    raise Exception(f"TissDB instance is not responsive: {e}")
Exception: TissDB instance is not responsive: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f4891795c10>: Failed to establish a new connection: [Errno 111] Connection refused'))

    ```

- **Step:** `Given a model and tokenizer`
  - **Feature:** `predict.feature`
  - **Scenario:** `Predicting the next token`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 249, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_kv_cache_steps.py", line 16, in context
    model = QuantaTissu(model_config)
            ^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/model.py", line 91, in __init__
    self.embeddings = Parameter(np.random.randn(vocab_size, d_model) / np.sqrt(d_model), name="embeddings")
                                ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "numpy/random/mtrand.pyx", line 1306, in numpy.random.mtrand.RandomState.randn
  File "numpy/random/mtrand.pyx", line 1466, in numpy.random.mtrand.RandomState.standard_normal
  File "numpy/random/_common.pyx", line 655, in numpy.random._common.cont
TypeError: 'NoneType' object cannot be interpreted as an integer

    ```

- **Step:** `Given a model and tokenizer`
  - **Feature:** `kv_cache.feature`
  - **Scenario:** `Generating with KV cache produces the same output as generating without it`
  - **Error:**
    ```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 249, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_kv_cache_steps.py", line 16, in context
    model = QuantaTissu(model_config)
            ^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/app/quanta_tissu/tisslm/model.py", line 91, in __init__
    self.embeddings = Parameter(np.random.randn(vocab_size, d_model) / np.sqrt(d_model), name="embeddings")
                                ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "numpy/random/mtrand.pyx", line 1306, in numpy.random.mtrand.RandomState.randn
  File "numpy/random/mtrand.pyx", line 1466, in numpy.random.mtrand.RandomState.standard_normal
  File "numpy/random/_common.pyx", line 655, in numpy.random._common.cont
TypeError: 'NoneType' object cannot be interpreted as an integer

    ```

- **Step:** `Given a running TissDB instance`
  - **Feature:** `integration.feature`
  - **Scenario:** `TissLM KnowledgeBase retrieves context from TissDB for a user prompt`
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
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f48917943e0>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 667, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f48917943e0>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/features/steps/test_database_steps.py", line 14, in running_tissdb_instance
    response = requests.get(f"{BASE_URL}/_health")
               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
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
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 700, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f48917943e0>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 249, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 17, in running_tissdb_instance
    raise Exception(f"TissDB instance is not responsive: {e}")
Exception: TissDB instance is not responsive: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f48917943e0>: Failed to establish a new connection: [Errno 111] Connection refused'))

    ```

- **Step:** `Given a running TissDB instance`
  - **Feature:** `integration.feature`
  - **Scenario:** `Sinew C++ client interaction pattern with TissDB`
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
urllib3.exceptions.NewConnectionError: <urllib3.connection.HTTPConnection object at 0x7f48917955e0>: Failed to establish a new connection: [Errno 111] Connection refused

The above exception was the direct cause of the following exception:

Traceback (most recent call last):
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 667, in send
    resp = conn.urlopen(
           ^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/connectionpool.py", line 841, in urlopen
    retries = retries.increment(
              ^^^^^^^^^^^^^^^^^^
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/urllib3/util/retry.py", line 519, in increment
    raise MaxRetryError(_pool, url, reason) from reason  # type: ignore[arg-type]
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
urllib3.exceptions.MaxRetryError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f48917955e0>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/features/steps/test_database_steps.py", line 14, in running_tissdb_instance
    response = requests.get(f"{BASE_URL}/_health")
               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
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
  File "/home/jules/.pyenv/versions/3.12.11/lib/python3.12/site-packages/requests/adapters.py", line 700, in send
    raise ConnectionError(e, request=request)
requests.exceptions.ConnectionError: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f48917955e0>: Failed to establish a new connection: [Errno 111] Connection refused'))

During handling of the above exception, another exception occurred:

Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 249, in run
    func(context, *match.groups())
  File "/app/tests/features/steps/test_database_steps.py", line 17, in running_tissdb_instance
    raise Exception(f"TissDB instance is not responsive: {e}")
Exception: TissDB instance is not responsive: HTTPConnectionPool(host='localhost', port=8080): Max retries exceeded with url: /_health (Caused by NewConnectionError('<urllib3.connection.HTTPConnection object at 0x7f48917955e0>: Failed to establish a new connection: [Errno 111] Connection refused'))

    ```
\n## Conclusion\n\nThe test run failed. See error details above.