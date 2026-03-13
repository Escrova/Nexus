# syntax=docker/dockerfile:1

FROM gcc:13-bookworm AS builder
WORKDIR /app

COPY include ./include
COPY src ./src
COPY CMakeLists.txt ./CMakeLists.txt

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --config Release -j"$(nproc)"

FROM debian:bookworm-slim
WORKDIR /app

COPY --from=builder /app/build/nexus /usr/local/bin/nexus

ENTRYPOINT ["/usr/local/bin/nexus"]
