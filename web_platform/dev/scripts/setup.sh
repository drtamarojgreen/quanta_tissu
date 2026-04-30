#!/bin/sh
# Build the High-Altitude Alpine Greenhouse image
docker build -t quantatissu-dev -f web_platform/dev/Dockerfile .
