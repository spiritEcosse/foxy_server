ARG BRANCH_NAME=dev
FROM ghcr.io/spiritecosse/foxy_server_build:${BRANCH_NAME} AS builder

COPY . /src
WORKDIR /src

RUN --mount=type=cache,target=/root/.cache/CPM \
    CPM_SOURCE_CACHE=/root/.cache/CPM \
    cmake --preset ninja-prod && cmake --build --preset ninja-prod

FROM debian:bookworm-slim AS runtime

ENV TZ=Europe/Madrid

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        ca-certificates \
        tzdata \
        libpq5 \
        libssl3 \
        zlib1g \
        wget \
        curl \
    && ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone \
    && rm -rf /var/lib/apt/lists/* \
    && curl -sSf https://atlasgo.sh | sh \
    && curl -fsSL https://claude.ai/install.sh | bash \
    && ln -s /root/.local/bin/claude /usr/local/bin/claude

# libc++ and libunistring runtime — binary links against libc++.so.1 and libunistring.so.5
RUN --mount=type=bind,from=builder,source=/usr/lib,target=/builder-lib \
    find /builder-lib -name "libc++.so.1*" -o -name "libc++abi.so.1*" -o -name "libunistring.so.5*" \
    | xargs -I{} cp {} /usr/lib/ && ldconfig

WORKDIR /app

COPY --from=builder /src/build/prod/foxy_server /app/foxy_server
COPY atlas.hcl /app/atlas.hcl
COPY migrations /app/migrations
RUN chmod +x /app/foxy_server

EXPOSE 8080

HEALTHCHECK --interval=30s --timeout=5s --start-period=10s --retries=3 \
    CMD wget -qO- http://localhost:8080/ || exit 1

CMD ["/app/foxy_server"]
