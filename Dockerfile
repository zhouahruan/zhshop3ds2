FROM devkitpro/devkitpro:latest

LABEL maintainer="3DS App Store"
LABEL description="Build environment for Nintendo 3DS homebrew app store"

WORKDIR /project

# Install extra 3DS libraries (jansson + libcurl already in base image usually)
RUN dkp-pacman -Syu --noconfirm && \
    dkp-pacman -S --noconfirm --needed \
        3ds-jansson \
        3ds-curl \
        3ds-libctru \
        3ds-citro3d \
        3ds-citro2d \
        3ds-zlib \
    && dkp-pacman -Scc --noconfirm

# Copy project files (cached layer if deps unchanged)
COPY . .

# Build the app.3dsx / app.smdh / app.cia
RUN make -j$(nproc) 2>&1 | tee build.log; exit ${PIPESTATUS[0]}

# Verify output exists
RUN ls -la build/app.cia build/app.3dsx build/app.smdh

# Default: copy build artifacts out
CMD ["sh", "-c", "mkdir -p /out && cp -v build/app.cia build/app.3dsx build/app.smdh /out/ 2>/dev/null; tail -f /dev/null"]
