# PlayTrack

PS5 homebrew (jailbroken console) that tracks playtime and trophies, PlayStation App style.

- `payload/` — ELF that runs on the console: reads `app.db` + trophy files (read-only) and serves `/api/v1` on port 7777.
- `server/` — Fastify + Postgres backend the console syncs to (in progress).
- `contract/` — JSON Schemas shared by both (in progress).

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
make                          # build/playtrack.elf
make deploy PS5_HOST=<ps5-ip> # needs elfldr listening on 9021 (after Y2JB)
```

Or through Prospero Manager (uploads to its payload library and launches it):

```sh
make prospero PS5_HOST=<ps5-ip>
``` A "PlayTrack ativo na porta 7777" toast confirms it started.

### UI and home-screen tile

The payload embeds two build outputs (via `payload/tools/embed.py`); rebuild them, then `make` again:

```sh
make -C payload ui     # ui/ (React + Vite, target safari13.1) -> ui/dist, served at /
make -C payload tile   # tile/ -> tile/dist/playtrack-tile.pkg (runs in Docker: PS5 PFS needs SHA3-256,
                       # which .NET on macOS lacks)
```

On first run the payload installs the tile (`PTRK00001`, Media tab, `deeplinkUri http://127.0.0.1:7777/`) through
`sceAppInstUtilAppInstallPkg`; `GET /api/v1/debug/tile` shows its state. The tile only opens the UI while the
payload is running, so keep the port at 7777. To change the tile's category or deeplink, delete it on the console
first: the home screen keeps the category from the first registration.

UI development against a payload: `make -C payload host-run` then `cd ui && npm run dev`
(`PLAYTRACK_API=http://<ps5-ip>:7777 npm run dev` to use the console).

### Endpoints

| Route | |
|---|---|
| `GET /api/v1/health` | version, capture time |
| `GET /api/v1/profile` | user + totals |
| `GET /api/v1/games` | games with playtime, last played, icon URL |
| `GET /api/v1/games/:titleId` | one game, with its `trophySetId` and `hidden` flag |
| `POST /api/v1/games/:titleId/hide` · `/unhide` | hide a title (e.g. homebrew) from the lists and totals; saved in `hidden.json` |
| `GET /api/v1/trophy-sets` | trophy sets with progress and counts per grade |
| `GET /api/v1/trophy-sets/:id` | one set |
| `GET /api/v1/trophy-sets/:id/trophies` | trophies: name, description, grade, hidden, earned, earnedAt, icon |
| `GET /api/v1/icons/:id` | PNG (game icons and trophy images) |
| `GET /api/v1/debug/*` | **only with `"debug": true` in config.json** (they expose console files on the LAN): |
| `GET /api/v1/debug/sources` | what the console has: app.db tables/columns/samples, trophy dirs, appmeta |
| `GET /api/v1/debug/stats` | uptime, counters, last error |
| `GET /api/v1/debug/log?lines=200` | recent log lines |
| `GET /api/v1/debug/ls?path=` / `debug/file?path=` | list / download allowlisted console paths |
| `GET /api/v1/debug/appdb` | consistent copy of app.db |
| `GET /api/v1/debug/tile` | home-screen tile: embedded, installed, state, error |
| `GET /` | the UI |

Config lives in `/data/playtrack/config.json` (created on first run: `port`, `language`, `debug`); logs in `/data/playtrack/playtrack.log`; hidden titles in `/data/playtrack/hidden.json`.

Console notifications: "PlayTrack ativo na porta 7777" on start; "PlayTrack instalado na aba Mídia" when the tile is first installed; "PlayTrack já está rodando" when loaded twice (the second copy exits); "a porta 7777 está em uso por outro app" if something else holds the port.

### Source layout

```
payload/src/
├── main.cpp      boot: privileges, config, routes, server
├── config.*      /data/playtrack/config.json
├── routes/       HTTP endpoints only, one file per resource (+ views.* for shared JSON shapes)
├── services/     business logic: playtime, trophies, library (cached join), icons, hidden, tile, console_layout
├── formats/      parsers for console files: app.db, TROPHY.UCP, TRPTITLE.DAT, npbind.dat
├── http/         generic HTTP: request/response types, router, server, loopback client
├── util/         fs, json, log, sha256, time, threads, crash handler, stats, byte readers
└── platform/     PS5 implementation vs host stand-in
```

Tests in `payload/tests/host/` follow the same layers: `test_util`, `test_http`, `test_formats`, `test_playtime`, `test_trophies`, `test_services`, `test_routes`, plus `test_readonly`, which checks that no route writes console data.

## License

GPL-3.0-or-later. Bundles SQLite (public domain) and cJSON (MIT, `payload/third_party/cjson/LICENSE`).
