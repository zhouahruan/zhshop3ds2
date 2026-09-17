FROM devkitpro/devkitarm:latest

LABEL maintainer="3DS App Store"
LABEL description="Build environment for Nintendo 3DS homebrew app store"

WORKDIR /project

# Install build dependencies for makerom
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        curl \
        build-essential \
        libyaml-dev \
        libelf-dev \
        zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

# Build and install makerom
RUN curl -sL https://github.com/3DSGuy/Project_CTR/archive/refs/heads/master.tar.gz | tar xz -C /tmp && \
    make -C /tmp/Project_CTR-master/makerom deps && \
    CC=gcc CXX=g++ make -C /tmp/Project_CTR-master/makerom && \
    cp /tmp/Project_CTR-master/makerom/bin/makerom /opt/devkitpro/tools/bin/makerom && \
    rm -rf /tmp/Project_CTR-master

# Note: devkitpro/devkitarm:latest pre-installs 3ds-jansson, 3ds-curl, libctru, citro3d, citro2d, and 3ds-zlib.
# dkp-pacman is omitted here to prevent Cloudflare 403 errors on CI runners when fetching from pkg.devkitpro.org.

# Copy project files
COPY . .

# Build app.3dsx, app.smdh, and app.cia
RUN bash -c "set -o pipefail; make cia 2>&1 | tee build.log"

# Verify output exists
RUN ls -la app.cia app.3dsx app.smdh

# Default: copy build artifacts out
CMD ["sh", "-c", "mkdir -p /out && cp -v app.cia app.3dsx app.smdh /out/ 2>/dev/null; tail -f /dev/null"]
