# QuantaTissu Platform

A modular, dependency-free web orchestration platform for the QuantaTissu ecosystem.

## How to Run

1.  **Ensure dependencies are met**:
    The platform requires Python 3 and standard libraries. Ensure `numpy` and `regex` are available for model inference.

2.  **Start the Server**:
    From the repository root, run the following command:
    ```bash
    python3 -m web_platform.backend.server
    ```

3.  **Access the Platform**:
    Once the server is running, open your web browser and navigate to:
    **[http://127.0.0.1:8000](http://127.0.0.1:8000)**

## Architecture
- **Backend**: Python `http.server` with modular domain handlers.
- **Frontend**: Vanilla HTML5, CSS3, and JavaScript modules.
- **Visualizations**: Native SVG and Canvas renderers.

## Proof of Access
To verify the server is responding correctly, you can run:
```bash
curl -I http://127.0.0.1:8000/
```
The server should respond with `HTTP/1.0 200 OK`.
