# syntax=docker/dockerfile:1

FROM gcc:13-bookworm AS builder
WORKDIR /app

RUN apt-get update \
    && apt-get install -y --no-install-recommends cmake \
    && rm -rf /var/lib/apt/lists/*

COPY include ./include
COPY src ./src
COPY CMakeLists.txt ./CMakeLists.txt

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --config Release -j"$(nproc)"

# Keep runtime on the same distro/toolchain family as builder to avoid
# libstdc++ ABI mismatches (e.g. missing GLIBCXX_* symbols).
FROM gcc:13-bookworm AS runtime
FROM debian:bookworm-slim
WORKDIR /app

COPY --from=builder /app/build/nexus /usr/local/bin/nexus

ENTRYPOINT ["/usr/local/bin/nexus"]
