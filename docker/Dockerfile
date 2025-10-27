FROM ubuntu:22.04

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy project files
COPY . .

# Build project
RUN mkdir -p build && cd build && \
    cmake .. && \
    cmake --build .

# Run tests by default
CMD ["ctest", "--test-dir", "build", "--output-on-failure"]
