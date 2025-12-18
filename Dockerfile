# Multi-arch Dockerfile for building DuckDB sqlite_scanner extension
# Supports linux_arm64 and linux_amd64

FROM ubuntu:22.04 AS builder

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    python3 \
    ccache \
    && rm -rf /var/lib/apt/lists/*

# Set up working directory
WORKDIR /workspace

# Copy the source code
COPY . .

# Initialize and update submodules
#RUN git submodule update --init --recursive

# Build the extension in release mode using Ninja for faster builds
RUN GEN=ninja make release

# The extension will be in build/release/extension/sqlite_scanner/
# Create a minimal output stage
FROM scratch AS export
COPY --from=builder /workspace/build/release/extension/sqlite_scanner/sqlite_scanner.duckdb_extension /sqlite_scanner.duckdb_extension
