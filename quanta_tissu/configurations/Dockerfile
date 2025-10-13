# ---- Builder Stage ----
# This stage is for cloning the repositories and installing build-time dependencies.
FROM ubuntu:22.04 AS builder

# Install build dependencies (git)
RUN apt-get update && \
    apt-get install -y --no-install-recommends git && \
    rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /app

# Clone the required repositories
RUN git clone https://github.com/drtamarojgreen/quanta_tissu.git
RUN git clone https://github.com/drtamarojgreen/quanta_haba.git

# ---- Final Stage ----
# This stage is for the final runtime image.
FROM ubuntu:22.04

# Set environment variables for non-interactive installation
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Etc/UTC
ENV PYTHONUNBUFFERED=1

# Create a non-root user
RUN useradd --create-home --shell /bin/bash appuser

# Install runtime dependencies (python, pip, tkinter)
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    python3 \
    python3-pip \
    python3-tk && \
    rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /home/appuser/app

# Copy the cloned repositories from the builder stage
COPY --from=builder /app /home/appuser/app

# --- IMPORTANT ---
# The following COPY commands require the 'docker build' command to be run
# from the PARENT directory of 'quanta_haba' and 'quanta_tissu'.
# The build context must contain the local 'quanta_tissu' directory.
#
# Assumed local directory structure:
# /path/to/your/workspace/  <-- Run 'docker build' from here
#  ├── quanta_haba/
#  │   └── Dockerfile      <-- This file
#  └── quanta_tissu/
#      ├── models/
#      ├── checkpoints/
#      └── data/
#
# These commands copy the specified local directories into the container,
# overwriting any corresponding folders from the git clone.
COPY quanta_tissu/models /home/appuser/app/quanta_tissu/models/
COPY quanta_tissu/checkpoints /home/appuser/app/quanta_tissu/checkpoints/
COPY quanta_tissu/data /home/appuser/app/quanta_tissu/data/

# Install Python dependencies for the Haba editor
RUN pip3 install --no-cache-dir -r /home/appuser/app/quanta_haba/src/p/requirements.txt

# Set the PYTHONPATH to the root of the app directory.
ENV PYTHONPATH=/home/appuser/app

# Change ownership of the app directory
RUN chown -R appuser:appuser /home/appuser/app

# Switch to the non-root user
USER appuser

# Set the final working directory to the Haba editor's root
WORKDIR /home/appuser/app/quanta_haba

# Command to run the editor
# Note: Running a GUI app from Docker requires specific 'docker run' options,
# such as sharing the X11 socket and setting the DISPLAY environment variable.
# Example:
# docker run -it --rm \
#   -e DISPLAY=$DISPLAY \
#   -v /tmp/.X11-unix:/tmp/.X11-unix \
#   <image_name>
CMD ["python3", "src/p/editor.py"]