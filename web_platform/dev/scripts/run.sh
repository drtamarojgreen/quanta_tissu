#!/bin/sh
# Run the greenhouse container in the background
docker run -d -p 8000:8000 --name quantatissu-dev-container quantatissu-dev
