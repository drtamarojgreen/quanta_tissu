# The High-Altitude Alpine Greenhouse 🏔️🌱

Welcome to the isolated dev environment for the QuantaTissu Framework. This directory contains the configuration to run the web framework and associated models within a lightweight Alpine Docker container.

## Architectural Metaphor

In the Greenhouse for Mental Health Development, we view this container as a **High-Altitude Alpine Greenhouse**. It is a protected, thin-air environment where only the most robust "plants" (code modules) can survive.

- **The Container (The Greenhouse)**: Provides the atmospheric pressure (OS dependencies) and nutrient-rich soil (Python packages) needed for the framework to thrive.
- **The CLI (Automated Irrigation & Pruning)**: The `cli.py` tool acts as the caretaker, allowing you to trigger growth (training), observe flowers (model generation), and maintain the soil (database operations) from the safety of your terminal.

## Technical Setup

### 1. Prerequisites
- Docker installed on your host machine.

### 2. Planting the Greenhouse (Setup)
Run the setup script to build the Docker image:
```bash
./web_platform/dev/scripts/setup.sh
```

### 3. Activating the Ecosystem (Run)
Start the container in the background:
```bash
./web_platform/dev/scripts/run.sh
```
The web platform will be accessible at `http://localhost:8000`.

### 4. Tending the Plants (CLI Usage)
Use the CLI tool to interact with the running framework:
```bash
./web_platform/dev/scripts/cli.sh --help
```

**Common Commands:**
- **Database Status**: `./web_platform/dev/scripts/cli.sh db status`
- **Model Generation**: `./web_platform/dev/scripts/cli.sh model "Tell me about neuroplasticity"`
- **Execute TissLang**: `./web_platform/dev/scripts/cli.sh tisslang "RUN 'ls'"`
- **List Tasks**: `./web_platform/dev/scripts/cli.sh tasks list`
- **Start Training**: `./web_platform/dev/scripts/cli.sh training start`

## Logging
All CLI actions and API responses are logged to `/var/log/framework.log` within the container, providing a detailed history of the greenhouse's evolution.

## Testing
To ensure the greenhouse's structural integrity, you can run the configuration tests:
```bash
PYTHONPATH=. python3 -m unittest web_platform/dev/tests/test_docker_config.py
```
