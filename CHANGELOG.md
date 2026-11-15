# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Changed

- The console uses the official server, `https://openpsapp.com`, when `server` in `config.json` is empty, so **Link account** works without editing the settings. Nothing is sent until a profile is linked.

## [0.2.0] - 2026-11-14

### Added

- Cloud sync. On the PS5, **Account → Link account** shows a code and a QR code; confirm it on the website and the console sends its profile, games, playtime, trophies and images on start, every 15 minutes, within about a minute after a change, and on **Sync now**. One console per account. Off until `server` is set in `config.json`.
- The payload's own HTTPS client, built on Mbed TLS 3.6.7, which trusts only the ISRG Root X1 and X2 certificates.
- The website (`web/`): sign-in, profile, trophies, game pages, search, account settings, and linking a console by typed code or camera QR scan.
- `contract/` (MIT) with the API, pairing and sync types, and [`SYNC.md`](SYNC.md), which documents everything the console sends.
- `core/`, the logic shared by the console UI and the website.
- `licenses.txt` with every bundled third-party license, served by the console at `/licenses.txt`.
- A new logo: a trophy inside an open ring.
- `make -C payload sdk`, which downloads ps5-payload-sdk v0.45 and checks its SHA-256.
- libFuzzer targets for every payload parser (`make -C payload fuzz`), run weekly in CI.
- Contributor docs (CONTRIBUTING, ARCHITECTURE, SECURITY, Code of Conduct and a README per area), CI, CodeQL, dependency review, OpenSSF Scorecard, Dependabot, and a release workflow with reproducible artifacts, SHA-256 sums and build provenance.

### Changed

- The README is now a user guide; build and development docs moved to CONTRIBUTING.md and the area READMEs.
- npm workspaces for `contract`, `core`, `ui`, `web` and `tile`, with one lockfile and root scripts (`build`, `dev:*`, `test`, `typecheck`, `lint`, `format`).
- ESLint, Prettier, EditorConfig and clang-format, applied to the whole codebase.
- The console UI imports its shared helpers from `@openpsapp/core`.
- API errors are snake_case codes: `{"error":"<code>"}`.
- The payload Makefile finds `llvm-config` on Linux too.
- The tile builder is `tile/builder/OpenPSAppTile.csproj`, and its .NET SDK image is pinned by digest.

### Removed

- The console UI's cloud mode. Clerk is no longer bundled in the ELF.

### Security

- `POST /api/v1/games/:id/hide` and `/unhide`, and every cloud action, require the `X-OpenPSApp: 1` header.
- Linking, unlinking and syncing work only from the console itself (loopback, with a matching `Host` header).
- Each connection has a 10 second request deadline.
- The payload answers only requests addressed to `localhost` or an IP address (`403 bad_host`), which blocks DNS rebinding from websites on your network.
- Fixed an integer overflow in the HTTPS client's chunked-response parser.
- A malformed timestamp in console data can no longer stop the sync thread.
- `cloud.json` is created with mode 0600, and CA certificate parse errors are checked.

### Fixed

- The host tests build on Linux.
- `+` in a URL path is no longer read as a space.
- Images that cannot be read are retried on the next sync instead of being marked as sent.
- Unlinking no longer waits for the network.
- The progress bars on the Trophies page line up across games.

## [0.1.0] - 2026-11-10

### Added

- Playtime per game and trophies, read from the console's files and shown in a web UI served by the payload on port 7777.
- A Media-tab tile, installed on the first run, that opens the UI.
- Trophy lists with filters, sorting and hidden trophies you can reveal one at a time.
- Hiding games from the lists and totals.
- Recent trophies per game and game key art.
- A monthly recap and a trophy timeline.
- The UI follows the console language, with English, Portuguese (Brazil) and Spanish, and a language picker. Console notifications are translated too.
- Diagnostic endpoints, off by default (`"debug": true`).
- Builds take their version from the git tag.

[Unreleased]: https://github.com/marcelorsdd/OpenPSApp/compare/v0.2.0...HEAD
[0.2.0]: https://github.com/marcelorsdd/OpenPSApp/compare/v0.1.0...v0.2.0
[0.1.0]: https://github.com/marcelorsdd/OpenPSApp/releases/tag/v0.1.0
