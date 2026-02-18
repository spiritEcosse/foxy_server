ARG BRANCH_NAME=dev
FROM ghcr.io/spiritecosse/foxy_server_build:${BRANCH_NAME} AS builder

COPY . /src
WORKDIR /src

RUN --mount=type=secret,id=env \
    export $(grep -v '^#' /run/secrets/env | grep -v '^$' | xargs) && \
    export CONFIG_APP_PATH=/app/config.json && \
    cmake --preset ninja-release && \
    cmake --build --preset ninja-release

FROM debian:bookworm-slim AS runtime

ENV TZ=Europe/Madrid

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        ca-certificates \
        tzdata \
        libpq5 \
        libssl3 \
        zlib1g \
        libuuid1 \
        libunwind8 \
        libbinutils \
    && ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /src/build/release/foxy_server /app/foxy_server
COPY docker/entrypoint.sh /app/entrypoint.sh
RUN chmod +x /app/entrypoint.sh /app/foxy_server

EXPOSE 8080

ENTRYPOINT ["/app/entrypoint.sh"]
CMD ["/app/foxy_server"]
