<p align="center"><img src="tile/sce_sys/icon0.png" width="128" alt="OpenPSApp icon"></p>

# OpenPSApp

Homebrew for a jailbroken PS5 that brings PlayStation App features to the console, offline: playtime per game, trophies (hidden ones included), recent trophies per game, a monthly recap and a trophy timeline. It runs as an ELF payload that serves a web UI on port 7777, opened from a tile in the Media tab. It only reads the console's playtime and trophy files; its own settings live in `/data/openpsapp`.

## Install

1. Download `openpsapp.elf` from the [latest release](https://github.com/marcelorsdd/OpenPSApp/releases/latest).
2. Load it with your payload loader (elfldr on port 9021, Prospero Manager or an autoload setup).
3. A notification confirms it started on port 7777. On first run it adds an OpenPSApp tile to the Media tab; open it to see your profile. From a phone or computer on the same network, open `http://<ps5-ip>:7777`.

Settings: `/data/openpsapp/config.json`.

## Repository

- `payload/`: the ELF (C++20, ps5-payload-sdk). Reads `app.db` and the trophy files and serves `/api/v1` plus the UI.
- `ui/`: the web UI (React, Vite, Tailwind), embedded into the ELF.
- `tile/`: the Media-tab tile PKG, embedded into the ELF.

## Payload

### Toolchain (macOS)

```sh
brew install llvm lld socat
mkdir -p ~/opt && curl -sSLO https://github.com/ps5-payload-dev/sdk/releases/download/v0.45/ps5-payload-sdk.zip
unzip -q ps5-payload-sdk.zip -d ~/opt        # -> ~/opt/ps5-payload-sdk
```

The Makefile defaults to `PS5_PAYLOAD_SDK=~/opt/ps5-payload-sdk` and finds Homebrew's `llvm-config` on its own.

### Run it on the Mac (no console)

```sh
cd payload
make host-test     # unit tests, incl. "no route writes console data"
make host-run      # same code, serves http://localhost:7777 over a fake console filesystem
curl localhost:7777/api/v1/games
```

`host-run` uses `tests/fixtures/real/root` when present (files pulled from your console), otherwise a synthetic fixture.

### Run it on the PS5

```sh
cd payload
make                          # build/openpsapp.elf
make deploy PS5_HOST=<ps5-ip> # needs elfldr listening on 9021 (after Y2JB)
```

Or through Prospero Manager (uploads to its payload library and launches it):

```sh
make prospero PS5_HOST=<ps5-ip>
```

A "OpenPSApp ativo na porta 7777" toast confirms it started.

### UI and home-screen tile

The payload embeds two build outputs (via `payload/tools/embed.py`); rebuild them, then `make` again:

```sh
make -C payload ui     # ui/ (React + Vite, target safari13.1) -> ui/dist, served at /
make -C payload tile   # tile/ -> tile/dist/openpsapp-tile.pkg (runs in Docker: PS5 PFS needs SHA3-256,
                       # which .NET on macOS lacks)
```

On first run the payload installs the tile (`OPSA00001`, Media tab, `deeplinkUri http://127.0.0.1:7777/`) through
`sceAppInstUtilAppInstallPkg`; `GET /api/v1/debug/tile` shows its state. The tile only opens the UI while the
payload is running, so keep the port at 7777. To change the tile's category or deeplink, delete it on the console
first: the home screen keeps the category from the first registration.

The UI is translated with i18next (`ui/src/i18n/locales/*.json`: pt-BR, en, es). By default it follows the console's language (reported by `/api/v1/health`); the header has a picker to choose one, saved in the browser. To add a language, add a locale file with the same keys and register it in `ui/src/i18n/index.ts`. A test fails if any locale is missing a key.

UI development against a payload: `make -C payload host-run` then `cd ui && npm run dev`
(`OPENPSAPP_API=http://<ps5-ip>:7777 npm run dev` to use the console).

### Endpoints

| Route | |
|---|---|
| `GET /api/v1/health` | version, capture time |
| `GET /api/v1/profile` | user + totals |
| `GET /api/v1/games` | games with playtime, last played, icon and key-art URLs |
| `GET /api/v1/games/:titleId` | one game, with its `trophySetId` and `hidden` flag |
| `POST /api/v1/games/:titleId/hide` · `/unhide` | hide a title (e.g. homebrew) from the lists and totals; saved in `hidden.json` |
| `GET /api/v1/trophy-sets` | trophy sets with progress and counts per grade |
| `GET /api/v1/trophy-sets/:id` | one set |
| `GET /api/v1/trophy-sets/:id/trophies` | trophies: name, description, grade, hidden, earned, earnedAt, icon |
| `GET /api/v1/trophies/earned?limit=N` | every earned trophy across all sets, newest first, with set and game (`limit` optional) |
| `GET /api/v1/icons/:id` | PNG (game icons, key art and trophy images) |
| `GET /api/v1/debug/*` | **only with `"debug": true` in config.json** (they expose console files on the LAN): |
| `GET /api/v1/debug/sources` | what the console has: app.db tables/columns/samples, trophy dirs, appmeta |
| `GET /api/v1/debug/stats` | uptime, counters, last error |
| `GET /api/v1/debug/log?lines=200` | recent log lines |
| `GET /api/v1/debug/ls?path=` / `debug/file?path=` | list / download allowlisted console paths |
| `GET /api/v1/debug/appdb` | consistent copy of app.db |
| `GET /api/v1/debug/tile` | home-screen tile: embedded, installed, state, error |
| `GET /` | the UI |

Config lives in `/data/openpsapp/config.json` (created on first run: `port`, `language` — `"auto"` follows the console's language, or a tag such as `"pt-BR"` — and `debug`); logs in `/data/openpsapp/openpsapp.log`; hidden titles in `/data/openpsapp/hidden.json`.

Console notifications: "OpenPSApp ativo na porta 7777" on start; "OpenPSApp instalado na aba Mídia" when the tile is first installed; "OpenPSApp já está rodando" when loaded twice (the second copy exits); "a porta 7777 está em uso por outro app" if something else holds the port.

### Source layout

```
payload/src/
├── main.cpp      boot: privileges, config, routes, server
├── config.*      /data/openpsapp/config.json
├── routes/       HTTP endpoints only, one file per resource (+ views.* for shared JSON shapes)
├── services/     business logic: playtime, trophies, library (cached join), icons, hidden, tile, console_layout
├── formats/      parsers for console files: app.db, TROPHY.UCP, TRPTITLE.DAT, npbind.dat
├── http/         generic HTTP: request/response types, router, server, loopback client
├── util/         fs, json, log, sha256, time, threads, crash handler, stats, byte readers
└── platform/     PS5 implementation vs host stand-in
```

Tests in `payload/tests/host/` follow the same layers: `test_util`, `test_http`, `test_formats`, `test_playtime`, `test_trophies`, `test_services`, `test_routes`, plus `test_readonly`, which checks that no route writes console data.

```
ui/src/
├── types/        API contract (JSON shapes) and UI types
├── services/     the only code that talks to the payload: typed endpoints, fetch, actions
├── hooks/        useApi, useStoredState, useRoute, useLanguageChoice, useDate
├── utils/        pure functions: format, dates, trophies, progress, router (tests sit next to them)
├── components/   ui/ (generic), trophies/, profile/, game/
├── pages/        one component per route
└── i18n/         i18next setup and locales
```

Imports use the `@/` alias for `ui/src`. Each endpoint in `services/api.ts` carries its response type, so `useApi(endpoints.profile)` is typed without a generic.

## Releasing

```sh
make -C payload ui tile        # rebuild the embedded UI and tile
git tag vX.Y.Z
make -C payload clean all      # build/openpsapp.elf, version taken from the tag
gh release create vX.Y.Z payload/build/openpsapp.elf ui/dist/licenses.txt --generate-notes
```

`GET /api/v1/health` reports the version the ELF was built from (`git describe --tags`).

## License

GPL-3.0-or-later (`LICENSE`). The payload links ps5-payload-sdk (GPL-3.0) and bundles SQLite (public domain), cJSON (MIT, `payload/third_party/cjson/LICENSE`) and a UI built with React, React DOM, i18next and Tailwind CSS (all MIT).

`make -C payload ui` writes `ui/dist/licenses.txt` with the license of every package that ends up in the bundle (`ui/plugins/licenses.ts`). The ELF serves it at `http://<ps5-ip>:7777/licenses.txt`, and each release attaches it.
