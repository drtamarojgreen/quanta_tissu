# Use a base image with Python and C++ development tools
FROM ubuntu:22.04

# Set environment variables
ENV PYTHONUNBUFFERED 1

# Install build-essential for C++ compilation, Python and pip
RUN apt-get update && \
    apt-get install -y build-essential python3.10 python3-pip cmake && \
    rm -rf /var/lib/apt/lists/*

# Set the working directory in the container
WORKDIR /app

# Copy requirements.txt and install Python dependencies
COPY requirements.txt /app/
RUN pip3 install --no-cache-dir -r requirements.txt

# Copy the entire project directory into the container
COPY . /app

# --- Build Tissdb ---
# Assuming Tissdb is a C++ project that uses CMake and its source is in /app/tissdb
WORKDIR /app/tissdb
RUN mkdir -p build && cd build && cmake .. && make
# Example: Copy the built executable to a common bin directory
# RUN cp build/tissdb_executable /usr/local/bin/

# --- Build tissu_sinew ---
# Assuming tissu_sinew is also a C++ project, and its source is in /app/tissu_sinew
WORKDIR /app/tissu_sinew
RUN mkdir -p build && cd build && cmake .. && make
# Example: Copy the built executable to a common bin directory
# RUN cp build/tissu_sinew_executable /usr/local/bin/

# --- Handle nexus_flow ---
# Assuming nexus_flow.exe is a pre-built Windows executable that needs to be copied.
# If it's a Linux executable, you might just need to ensure it's executable.
# If it needs to be built from source on Linux, specific instructions are required.
# For now, it's copied as part of the initial 'COPY . /app' step.
# If it's a Linux binary and needs to be executable:
# RUN chmod +x /app/nexus_flow.exe

# --- Connect everything in pipelines (Post-build steps) ---
# This section would involve any steps to integrate the built components,
# e.g., setting up environment variables, creating configuration files,
# or moving executables to a common PATH.

# Reset WORKDIR to root of the application
WORKDIR /app

# Command to run the application (example, adjust as needed)
# This would typically be the main entry point of your application,
# which might depend on the built components.
# CMD ["python3", "quanta_tissu/main.py"] # Example if there's a main Python script