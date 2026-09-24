FROM ubuntu:24.04

ARG TARGETARCH
ARG BAZELISK_VERSION=1.29.0
ARG LOOM_UID=1000
ARG LOOM_GID=1000

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
      ca-certificates \
      clang-19 \
      clang-format-19 \
      clang-tidy-19 \
      curl \
      llvm-19 \
      python3 \
    && rm -rf /var/lib/apt/lists/*

# Pin Bazelisk and verify its release artifact before installing it.
RUN set -eu; \
    case "$TARGETARCH" in \
      amd64) deb_arch=amd64; expected=186d78a20e1a64f59ba08987791a989892d142c9d3a9f9cc0c5c35e201f53924 ;; \
      arm64) deb_arch=arm64; expected=db8ada89c841afd2cb33db7d13aa98ea4fb14612579a8bae84722250caa84272 ;; \
      *) echo "Unsupported architecture: $TARGETARCH" >&2; exit 1 ;; \
    esac; \
    curl --fail --location --silent --show-error \
      "https://github.com/bazelbuild/bazelisk/releases/download/v${BAZELISK_VERSION}/bazelisk-${deb_arch}.deb" \
      --output /tmp/bazelisk.deb; \
    echo "$expected  /tmp/bazelisk.deb" | sha256sum --check --status; \
    apt-get update; \
    apt-get install -y --no-install-recommends /tmp/bazelisk.deb; \
    ln -s /usr/bin/bazelisk /usr/local/bin/bazel; \
    rm -f /tmp/bazelisk.deb; \
    rm -rf /var/lib/apt/lists/*

RUN mkdir -p /home/loom/.cache/bazel \
    && chown -R "$LOOM_UID:$LOOM_GID" /home/loom

ENV HOME=/home/loom
USER ${LOOM_UID}:${LOOM_GID}
WORKDIR /workspace
CMD ["./scripts/check.sh"]
