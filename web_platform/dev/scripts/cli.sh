#!/bin/sh
# Execute the CLI tool within the running greenhouse container
docker exec -it quantatissu-dev-container python /app/web_platform/dev/cli.py "$@"
