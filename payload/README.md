# Payload

The ELF that runs on the PS5: C++20, built with [ps5-payload-sdk](https://github.com/ps5-payload-dev/sdk) v0.45. It reads the console's playtime database and trophy files, serves a JSON API and the embedded [console UI](../ui/README.md) on port 7777, installs the [Media-tab tile](../tile/README.md), and, when a server is configured, syncs to the cloud over its own HTTPS client.

Setup and the general rules are in [CONTRIBUTING.md](../CONTRIBUTING.md). How the payload fits with the rest is in [ARCHITECTURE.md](../ARCHITECTURE.md).

## Make targets

| Command                                      | What it does                                                                                              |
| -------------------------------------------- | --------------------------------------------------------------------------------------------------------- |
| `make sdk`                                   | Downloads ps5-payload-sdk v0.45 into `PS5_PAYLOAD_SDK`, checks its SHA-256 and writes a version file.      |
| `make` (`make all`)                          | Builds `build/openpsapp.elf`.                                                                             |
| `make host-test`                             | Builds and runs the 105 host tests.                                                                       |
| `make host-run`                              | Runs the same code on your computer at `http://localhost:7777`, over a fake console filesystem.           |
| `make ui`                                    | Runs `npm ci` at the root, then builds `@openpsapp/ui` into `ui/dist`.                                    |
| `make tile`                                  | Runs `tile/build.sh`, which writes `tile/dist/openpsapp-tile.pkg`.                                        |
| `make deploy PS5_HOST=192.168.1.50`          | Sends the ELF to elfldr (port `PS5_PORT`, default 9021).                                                  |
| `make prospero PS5_HOST=192.168.1.50`        | Uploads the ELF to Prospero Manager's payload library and launches it (port `PROSPERO_PORT`, default 7070). |
| `make fuzz`                                  | Builds the libFuzzer targets in `tests/fuzz/` with ASan and UBSan and runs each for `FUZZ_SECONDS` (60).  |
| `make clean`                                 | Removes `build/`, `build-dev/` and the generated synthetic fixture.                                       |

Variables:

| Variable              | Default                     | Use                                                                                                    |
| --------------------- | --------------------------- | ------------------------------------------------------------------------------------------------------ |
| `PS5_PAYLOAD_SDK`     | `~/opt/ps5-payload-sdk`     | Where the SDK lives. `sdk-check` stops the build if it is missing or its version file is not v0.45.    |
| `LLVM_CONFIG`         | found automatically         | The `llvm-config` to use. The Makefile tries Homebrew's, then `llvm-config-23` down to `-18`, then `llvm-config`. |
| `HOST_CXX`, `HOST_CC` | `c++`, `cc`                 | Compilers for the host build and tests.                                                                |
| `DEV_CA`              | empty                       | A PEM file to trust besides the pinned roots, for testing against a local server. Builds into `build-dev/` with a `+devca` version. |
| `FIXTURE_ROOT`        | `tests/fixtures/real/root` if present, else the synthetic fixture | The fake console root for `host-run`.                                    |
| `FUZZ_SECONDS`        | `60`                        | How long `make fuzz` runs each target.                                                                 |
| `FUZZ_MAX_LEN`        | `65536`                     | The largest input libFuzzer generates.                                                                 |
| `FUZZ_CXX`, `FUZZ_CC` | Homebrew LLVM, else `clang++`, `clang` | Compilers with libFuzzer (Apple clang has none).                                            |
| `FUZZ_CORPUS`, `FUZZ_ARTIFACTS` | `build/fuzz/corpus`, `build/fuzz/artifacts` | Where the grown corpus and any crash files go.                                 |

`host-run` also reads `OPENPSAPP_USER` (default `1eb71bbd`), `OPENPSAPP_USER_NAME`, `OPENPSAPP_LANG_ID`, `OPENPSAPP_ACCOUNT_ID` and `OPENPSAPP_DEVICE_ID` to fake the signed-in user and console. `tests/fixtures/` is ignored by git: a real fixture copied from a console holds personal data and must never be committed.

## API

Every response is JSON unless noted. Errors are `{"error":"<code>"}` with a snake_case code. `HEAD` works wherever `GET` does.

| Route                                        | Returns                                                                                                         |
| -------------------------------------------- | --------------------------------------------------------------------------------------------------------------- |
| `GET /api/v1/health`                         | `ok`, `version`, `source` (`"console"`), `capturedAt`, `language`                                               |
| `GET /api/v1/profile`                        | `user` (`id`, `name`) and `totals`: games, playtime, earned trophies per grade, earned and total                |
| `GET /api/v1/games`                          | Games with playtime, last played date, icon and key art URLs, trophy set and `hidden`                           |
| `GET /api/v1/games/:titleId`                 | One game                                                                                                        |
| `POST /api/v1/games/:titleId/hide`, `/unhide` | Hides or shows a game in lists and totals and returns it. Needs `X-OpenPSApp: 1`. Saved in `hidden.json`.      |
| `GET /api/v1/trophy-sets`                    | Trophy sets with progress, earned and total, counts per grade, last earned date                                 |
| `GET /api/v1/trophy-sets/:id`                | One trophy set                                                                                                  |
| `GET /api/v1/trophy-sets/:id/trophies`       | Its trophies: name, description, grade, hidden, earned, earnedAt, icon URL                                      |
| `GET /api/v1/trophies/earned?limit=N`        | Every earned trophy across sets, newest first, with its set and game (`limit` optional)                         |
| `GET /api/v1/icons/:id`                      | PNG: game icons, key art and trophy images                                                                      |
| `GET /api/v1/cloud`                          | Cloud status: `configured`, `local`, `linked`, `syncing`, `profileName`, `lastSyncAt`, `lastError`, `pairing`. `pairing.userCode` and `pairing.verificationUri` appear only for loopback requests; others get only `pairing.expiresIn`. |
| `POST /api/v1/cloud/link`                    | Asks the server for a pairing code. Console only (see below).                                                   |
| `POST /api/v1/cloud/unlink`                  | Cancels a pairing code, or unlinks the profile and queues its token for revocation. Console only.              |
| `POST /api/v1/cloud/sync`                    | Syncs now. Console only.                                                                                        |
| `GET /`, `GET /:file`, `GET /assets/:file`   | The embedded UI, plus `licenses.txt`                                                                            |

Only with `"debug": true` in `config.json`. These expose console files and ids to the network:

| Route                                          | Returns                                                                             |
| ---------------------------------------------- | ----------------------------------------------------------------------------------- |
| `GET /api/v1/debug/sources`                    | What the console has: `app.db` tables, columns and samples, trophy folders, appmeta |
| `GET /api/v1/debug/stats`                      | Version, platform, uptime, request and collection counters, duration of the last collection |
| `GET /api/v1/debug/log?lines=200`              | The last log lines (1 to 500), as text                                              |
| `GET /api/v1/debug/ls?path=`, `/debug/file?path=` | Lists or downloads an allowlisted console path                                   |
| `GET /api/v1/debug/appdb`                      | A consistent copy of `app.db`                                                       |
| `GET /api/v1/debug/tile`                       | The tile: embedded, installed, state, error                                         |
| `GET /api/v1/debug/identity`                   | The signed-in user, account and console ids, and whether each trophy file matches them |

### Request rules

- **`Host` allowlist:** every request needs a `Host` of `localhost` or an IP address (`127.0.0.1:7777`, `192.168.1.50:7777`, `[::1]:7777`), or it gets `403 bad_host`. Host names are refused, which blocks DNS rebinding: a website cannot point its own name at the console and read the API.
- **`POST` routes** need the header `X-OpenPSApp: 1`, or they answer `403 forbidden`. The payload sends no CORS headers, so pages on other origins cannot send it.
- **Cloud actions** (`/api/v1/cloud/link`, `/unlink`, `/sync`) also need a loopback peer and a `Host` of `127.0.0.1:<port>` or `localhost:<port>`, or they answer `403 console_only`. Without `server` in `config.json` they answer `409 not_configured`.
- **Limits:** 16 KiB of headers (`431 headers_too_large`), 64 KiB of body (`413 too_large`), a 10 s deadline to receive each request, and 32 connections at once (`503 busy`).

Error codes: `bad_host`, `not_found`, `method_not_allowed`, `bad_request`, `headers_too_large`, `too_large`, `busy`, `internal`, `forbidden`, `console_only`, `not_configured`, `no_profile`, `already_linked`, `cloud_unavailable`, `save_failed`, `playtime_unavailable`, `path_not_allowed`, `appdb_copy_failed`.

## Configuration

`/data/openpsapp/config.json` is created on the first run. Changes apply on the next start.

| Key        | Type    | Default  | Meaning                                                                                                   |
| ---------- | ------- | -------- | --------------------------------------------------------------------------------------------------------- |
| `port`     | number  | `7777`   | HTTP port. The tile opens the port in `tile/sce_sys/param.json` (7777); the payload logs a mismatch.      |
| `language` | string  | `"auto"` | `"auto"` follows the console's system language; otherwise a tag such as `"pt-BR"`. Used for notifications, trophy texts and the UI's default. |
| `debug`    | boolean | `false`  | Registers the `/api/v1/debug/*` routes.                                                                   |
| `server`   | string  | `""`     | The cloud server's `https://` address. Empty disables the cloud thread and makes `/api/v1/cloud` answer `configured: false`. |

Other files in `/data/openpsapp`:

| File                | Content                                                                             |
| ------------------- | ----------------------------------------------------------------------------------- |
| `openpsapp.log`     | The log                                                                             |
| `hidden.json`       | Hidden title ids                                                                    |
| `cloud.json`        | The link per local user: device token, device id, profile name, tokens to revoke. Mode 0600. |
| `asset-hashes.json` | A cache of image SHA-256 hashes, so images are not hashed again on every sync       |
| `tile.pkg`          | Written only while the tile installs, then removed                                  |
| `app.db.copy`       | Written by `/api/v1/debug/appdb`                                                    |

## Notifications

The payload sends console notifications in the configured language: Portuguese for `pt*`, Spanish for `es*`, English otherwise (`src/services/messages.cpp`).

| When                          | English                                       | Portuguese                                         | Spanish                                          |
| ----------------------------- | --------------------------------------------- | -------------------------------------------------- | ------------------------------------------------ |
| It started                    | OpenPSApp running on port 7777                | OpenPSApp ativo na porta 7777                      | OpenPSApp activo en el puerto 7777               |
| The tile was installed        | OpenPSApp added to the Media tab              | OpenPSApp instalado na aba Mídia                   | OpenPSApp añadido a la pestaña Multimedia        |
| A copy already runs           | OpenPSApp is already running                  | OpenPSApp já está rodando                          | OpenPSApp ya se está ejecutando                  |
| Something else holds the port | OpenPSApp: port 7777 is used by another app   | OpenPSApp: a porta 7777 está em uso por outro app  | OpenPSApp: el puerto 7777 está en uso por otra app |

When the port is taken, the payload asks `/api/v1/health` on it; if another OpenPSApp answers, the new copy exits quietly after the "already running" notification.

## Source layout

```
payload/
├── src/
│   ├── main.cpp      boot: privileges, config, routes, server, tile, cloud
│   ├── config.*      /data/openpsapp/config.json
│   ├── routes/       HTTP endpoints only, one file per resource; views.* holds shared JSON shapes
│   ├── services/     playtime, trophies, library (cached join), icons, hidden, tile, messages, diagnostics, console_layout
│   ├── formats/      parsers for console files: app.db, TROPHY.UCP, TRPTITLE.DAT, npbind.dat
│   ├── cloud/        pairing and sync: controller, client, snapshot, store (cloud.json), image hashes, service thread
│   ├── net/          HTTPS client on mbedTLS, URL parsing, HTTP/1.1 messages, pinned CA roots, mbedtls_config.h
│   ├── http/         HTTP server, router, request and response types, loopback client
│   ├── util/         fs, json, log, sha256, time, threads, crash handler, stats, byte readers
│   └── platform/     platform.hpp, with ps5.cpp for the console and host.cpp for tests and host-run
├── tests/
│   ├── host/         the host test suite
│   └── fuzz/         libFuzzer targets, their seed generator and the tracked seeds in `seeds/`
├── tools/embed.py    turns ui/dist, the tile PKG and DEV_CA into generated C++
└── third_party/      vendored SQLite, cJSON and Mbed TLS (see third_party/README.md)
```

Tests in `tests/host/` follow the same layers: `test_util`, `test_http`, `test_net`, `test_formats`, `test_playtime`, `test_trophies`, `test_services`, `test_messages`, `test_routes`, `test_cloud`, plus `test_readonly`, which checks that no route writes console data. `fixture.cpp` generates the synthetic console filesystem (`openpsapp-tests --make-fixture <dir>`), and `tls_fixture.cpp` runs a local TLS server for the HTTPS client tests (host builds define `OPENPSAPP_TLS_TEST_SERVER` for that).

## Versions and reproducible builds

- The version comes from `git describe --tags --always --dirty` and is written to `build/gen/version.hpp`. A build outside a git checkout, such as from a source tarball, reports `dev`. `DEV_CA` builds add `+devca`. `GET /api/v1/health` returns it.
- The build has no timestamps: `embed.py` sorts the UI files and gzips them with a zero mtime, and the code does not use `__DATE__` or `__TIME__`.
- The [release workflow](../.github/workflows/release.yml) builds the ELF twice from the same inputs and fails if the SHA-256 differs.
- To rebuild a release yourself, use the same toolchain (Ubuntu 24.04 with `clang-18`, `lld-18` and `llvm-18`, SDK v0.45) and the same inputs: check out the tag, run `npm ci && npm run build:ui`, put the release's `openpsapp-tile.pkg` in `tile/dist/`, then `make -C payload LLVM_CONFIG=llvm-config-18` and compare with `SHA256SUMS`.

## Third-party code

`third_party/` holds SQLite 3.54.1, cJSON 1.7.19 and Mbed TLS 3.6.7. Versions, licenses and the update procedure are in [third_party/README.md](third_party/README.md).
