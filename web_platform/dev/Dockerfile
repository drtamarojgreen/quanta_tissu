# High-Altitude Alpine Greenhouse: Dev Environment for QuantaTissu
# Provides a lightweight, isolated ecosystem for framework development.

FROM python:3.12-alpine

# Root System: Install essential system dependencies
# build-base and python3-dev for potential native extensions
# py3-numpy and py3-requests for efficient pre-compiled packages
RUN apk add --no-cache \
    build-base \
    python3-dev \
    py3-numpy \
    py3-requests \
    libffi-dev

# Visible Foliage: Python requirements
WORKDIR /app
COPY requirements.txt .
RUN pip install --no-cache-dir regex

# Environment Architecture
ENV PYTHONPATH=/app
ENV TISSDB_HOST=localhost
ENV TISSDB_PORT=9876

# Deploy the Greenhouse: Copy only source code directories
COPY analytics /app/analytics
COPY integrations /app/integrations
COPY lite /app/lite
COPY quanta_tissu /app/quanta_tissu
COPY tisslm /app/tisslm
COPY web_platform /app/web_platform

# Setup Logging
RUN mkdir -p /var/log && touch /var/log/framework.log && chmod 666 /var/log/framework.log

EXPOSE 8000

# Continuous Growth: Start the web platform server
CMD ["python3", "-m", "web_platform.backend.server"]
