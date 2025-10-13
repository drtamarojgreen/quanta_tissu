# Base image with Python and C++ development tools
FROM ubuntu:22.04

# Set environment variables
ENV PYTHONUNBUFFERED 1

# Install dependencies for C++ compilation and Python
RUN apt-get update && \
    apt-get install -y build-essential python3.10 python3-pip && \
    rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /app

# Copy and install Python dependencies
COPY requirements.txt /app/
RUN pip3 install --no-cache-dir -r requirements.txt

# Copy the entire project directory into the container
COPY . /app

# --- Build C++ Components ---

# Build tissdb (using its Makefile)
# The Makefile also handles compiling tissu_sinew.cpp
RUN make -C tissdb
RUN cp tissdb/tissdb /usr/local/bin/

# Build nexus_flow (manual compilation)
RUN g++ -std=c++17 -o nexus_flow quanta_tissu/nexus_flow/main.cpp quanta_tissu/nexus_flow/graph_logic.cpp quanta_tissu/nexus_flow/json/json.cpp -Iquanta_tissu/nexus_flow/json
RUN cp nexus_flow /usr/local/bin/

# Reset WORKDIR to the application root
WORKDIR /app

# Define a default command (placeholder)
CMD ["/bin/bash"]