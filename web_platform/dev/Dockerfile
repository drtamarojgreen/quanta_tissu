# High-Altitude Alpine Greenhouse: Dev Environment for QuantaTissu
# Provides a lightweight, isolated ecosystem for framework development.

FROM python:3.12-alpine

# Root System: Install essential system dependencies and Git
RUN apk add --no-cache \
    build-base \
    python3-dev \
    py3-numpy \
    py3-requests \
    libffi-dev \
    git

# Deploy the Greenhouse: Clone the source code from GitHub
WORKDIR /app
RUN git clone https://github.com/drtamarojgreen/quanta_tissu/ .

# Visible Foliage: Python requirements
RUN pip install --no-cache-dir regex

# Refresh the Dev Tools: Copy local dev configuration into the cloned repo
COPY web_platform/dev /app/web_platform/dev

# Environment Architecture
ENV PYTHONPATH=/app
ENV TISSDB_HOST=localhost
ENV TISSDB_PORT=9876

# Setup Logging
RUN mkdir -p /var/log && touch /var/log/framework.log && chmod 666 /var/log/framework.log

EXPOSE 8000

# Continuous Growth: Start the web platform server
CMD ["python3", "-m", "web_platform.backend.server"]
