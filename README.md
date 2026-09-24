# 3DS App Store

A native Nintendo 3DS homebrew application store, built with **C** + **devkitPro** + **citro3d/citro2d** + **libcurl**. Final product is an installable `.cia` file that runs on real 3DS hardware.

## Features

- Dual-screen UI (top 400×240, bottom 320×240 touch)
- eShop-style layout: Banner carousel, app grid, detail pages, downloads
- libcurl HTTPS networking (api endpoints pluggable)
- 3D scene transition animations
- Forum + Chat modules
- Local CIA install via `am:am` service

## Project Structure

```
/workspace
├── Makefile               # devkitPro build rules
├── Dockerfile             # docker build → app.cia
├── .github/workflows/     # CI builds and releases
├── resources/             # PNG icons, banners, fonts
├── romfs/                 # files packed into CIA
└── source/
    ├── main.c             # entry + main loop
    ├── core/              # render, input, scene manager, transitions
    ├── net/               # libcurl http wrapper + API client (mock switch)
    ├── data/              # types, global store, JSON parser
    ├── ui/                # widget, button, grid, list, carousel, modal ...
    ├── scenes/            # home / applist / detail / downloads / forum / chat / settings
    └── download/          # downloader + CIA installer
```

## Build Locally

### Option A — Docker (recommended, no devkitPro install needed)

```bash
docker build -t 3ds-appstore .
mkdir -p out
docker run --rm -v "$PWD/out:/out" 3ds-appstore
# -> out/app.cia  out/app.3dsx  out/app.smdh
```

### Option B — Native devkitPro (already installed)

```bash
export DEVKITARM=/opt/devkitpro/devkitARM
export DEVKITPRO=/opt/devkitpro
make -j$(nproc)
# -> build/app.cia  build/app.3dsx  build/app.smdh
```

### Option C — GitHub Actions CI

Push to GitHub. The workflow in `.github/workflows/build.yml` will build on every push and upload artifacts (downloadable from the Actions tab). Tag a release like `v1.0` to auto-draft a GitHub Release with the CIA attached.

## Install on Real 3DS

1. Copy `app.cia` to the SD card root (or `/cias/`).
2. Launch **FBI** (install via Homebrew Launcher if missing).
3. Navigate to `SD` → find `app.cia` → Install.
4. Return to HOME Menu — the 3DS App Store icon appears and launches.

For 3DSX-only testing without CIA install: copy `app.3dsx` + `app.smdh` to `/3ds/3DSAppStore/` on SD and launch via Homebrew Launcher.

## Configure API Endpoints

Edit `source/net/api_config.h`:

```c
#define USE_MOCK          1            // 1: use built-in mock data; 0: hit real API
#define API_BASE_URL      "https://your-api.example.com/api/v1"
#define API_TIMEOUT_MS    10000
#define USER_TOKEN         ""           // optional bearer token
```

Set `USE_MOCK 0` and replace `API_BASE_URL` with your real endpoint, then `make` again. The `ApiClient` interface in `source/net/api.h` documents every endpoint shape; backend should match.

## Test in Emulator

After build, load `build/app.3dsx` in **Lime3DS** or **Citra**:
- 3D rendering works
- Touch (mouse) on bottom screen
- Keyboard remaps to 3DS buttons

Network in emulator may need `Allow CPU JIT` and configured DNS.

## API Contract Summary

See [source/net/api.h](source/net/api.h) for full signatures. Quick reference:

| Endpoint                          | Purpose              |
|-----------------------------------|----------------------|
| GET /recommend                    | Home banner + picks  |
| GET /categories                   | Category tabs        |
| GET /apps?category=&keyword=&sort=&page= | App list, paginated |
| GET /apps/{id}                    | Detail incl. screenshots |
| GET /apps/{id}/download           | Returns CIA URL + size |
| GET /forum/posts?page=            | Forum list           |
| GET /forum/posts/{id}             | Post + replies       |
| POST /forum/posts                 | New post             |
| POST /forum/posts/{id}/replies    | Reply                |
| GET /chat/sessions                | Chat session list    |
| GET /chat/sessions/{id}/messages  | Message list         |
| POST /chat/sessions/{id}/messages | Send message         |

Mock responses are in `source/net/mock/*.mock.c` — extend them while you build the real backend.

## License

MIT — use freely. Generated CIA and source are yours.
