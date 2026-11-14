# Contributing to OpenPSApp

Thanks for helping. This guide covers the setup for each part of the repository, the checks CI runs, the rules, the pull request flow and how releases are made. For how the pieces fit together, read [ARCHITECTURE.md](ARCHITECTURE.md) first.

Everyone taking part follows the [Code of Conduct](CODE_OF_CONDUCT.md). Report vulnerabilities privately, as described in [SECURITY.md](SECURITY.md), not in issues or pull requests.

For anything larger than a small fix, open an issue first so we can agree on the approach.

## Repository layout

| Path                       | What it is                                                                       | Details                                              |
| -------------------------- | -------------------------------------------------------------------------------- | ---------------------------------------------------- |
| `payload/`                 | The ELF that runs on the PS5 (C++20, ps5-payload-sdk)                            | [payload/README.md](payload/README.md)               |
| `ui/`                      | The console UI (React, Tailwind 3), embedded in the ELF, runs on Safari 13.1     | [ui/README.md](ui/README.md)                         |
| `web/`                     | The website (React 19, Tailwind 4, shadcn), served by the cloud server           | [web/README.md](web/README.md)                       |
| `core/`                    | `@openpsapp/core`, pure TypeScript shared by `ui/` and `web/`                    | [core/README.md](core/README.md)                     |
| `contract/`                | `@openpsapp/contract`, the API and sync types (MIT), copied by the server        | [contract/README.md](contract/README.md)             |
| `tile/`                    | The Media-tab tile PKG, embedded in the ELF                                      | [tile/README.md](tile/README.md)                     |
| `tools/`                   | Build helpers shared by the Vite apps                                            | [tools/README.md](tools/README.md)                   |
| `payload/third_party/`     | Vendored SQLite, cJSON and Mbed TLS                                              | [payload/third_party/README.md](payload/third_party/README.md) |
| `SYNC.md`                  | What the console sends to the cloud                                              |                                                      |

The cloud server is closed source and lives in a separate private repository. `contract/` and `SYNC.md` are its public description.

## Setup

### TypeScript parts (`contract`, `core`, `ui`, `web`, `tile`)

- **Node 24** (see `.nvmrc`; `nvm use` picks it up) with the npm that ships with it.
- The root `package.json` declares npm workspaces for `contract`, `core`, `ui`, `web` and `tile`, with one lockfile at the root. Install everything from the root:

  ```sh
  npm ci
  ```

- Add or update a dependency from the root, pinned to an exact version:

  ```sh
  npm install --save-exact <package>@<version> -w @openpsapp/web
  ```

  Never run `npm install` inside a workspace folder; it would create a second lockfile.

### Payload

- **LLVM with clang and lld**, version 18 or newer. The Makefile finds Homebrew's `llvm-config` and `llvm-config-23` down to `llvm-config-18`; set `LLVM_CONFIG` to choose one.
  - macOS: `brew install llvm lld socat`
  - Debian/Ubuntu: `sudo apt-get install clang-18 lld-18 llvm-18 python3 make curl unzip socat`
- **python3** and **make**. `curl`, `unzip` and `shasum` for `make sdk`; `socat` for `make deploy`.
- **ps5-payload-sdk v0.45.** Install it with:

  ```sh
  make -C payload sdk                                    # into ~/opt/ps5-payload-sdk
  make -C payload sdk PS5_PAYLOAD_SDK=/path/to/sdk       # anywhere else
  ```

  `make sdk` downloads the release, checks its SHA-256 and writes a `.openpsapp-sdk-version` file into the SDK. Every build runs `sdk-check`, which stops if the SDK is missing or its version file names another release. Pass the same `PS5_PAYLOAD_SDK=` to every `make` if you did not use the default.

- **Host tests** need only a C++20 compiler (clang or gcc). On Linux, glibc 2.36 or newer is needed for `arc4random_buf` (Ubuntu 24.04 is fine).
- **clang-format 23.x** for the C++ format check. On macOS it comes with `brew install llvm`; elsewhere, the `clang-format` package on PyPI has the same version.

### Tile

`tile/build.sh` builds the PKG with [LibProsperoPKG](https://github.com/SvenGDK/LibProsperoPKG) at a pinned commit, using either:

- **Docker** (the default), with the digest-pinned `mcr.microsoft.com/dotnet/sdk:10.0.402` image; or
- a local **.NET SDK 10** with `TILE_LOCAL_DOTNET=1`. This works on Linux. On macOS use Docker: the PS5 package format needs SHA3-256, which .NET on macOS lacks.

## Checks

These are the commands CI runs. Run the ones for every part you touch before opening a pull request.

| Part                    | Commands                                                                                                                                                                       |
| ----------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| All TypeScript and YAML | `npm run lint`, `npm run format:check`                                                                                                                                         |
| Website and core        | `npm run typecheck -w @openpsapp/contract -w @openpsapp/core -w @openpsapp/web`, `npm test -w @openpsapp/core -w @openpsapp/web`, `npm run build:web`                          |
| Console UI              | `npm run typecheck -w @openpsapp/ui`, `npm test -w @openpsapp/ui`, `npm run build:ui`, then `grep -ril clerk ui/dist` must find nothing                                        |
| Payload tests           | `make -C payload host-test`                                                                                                                                                    |
| Payload format          | `find payload/src payload/tests/host payload/tests/fuzz \( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' \) -print0 \| xargs -0 clang-format --dry-run --Werror`                                  |
| Payload ELF             | `make -C payload` (after `npm run build:ui` and, for the tile, `tile/build.sh`)                                                                                                |
| Tile                    | `tile/build.sh` (or `TILE_LOCAL_DOTNET=1 tile/build.sh`)                                                                                                                       |

Shortcuts: `npm test` and `npm run typecheck` run every workspace that has the script; `npm run format` rewrites TypeScript, JSON, CSS and YAML with Prettier; `clang-format -i <file>` fixes C++.

## Running things

**The payload on your computer.** `make -C payload host-run` serves the same code on `http://localhost:7777` over a fake console filesystem (`payload/tests/fixtures/real/root` if you have one, otherwise a generated synthetic fixture).

**The console UI against it.** In another terminal, `npm run dev:ui`. Vite proxies `/api` to `http://localhost:7777`; point it at a real console with `OPENPSAPP_API=http://192.168.1.50:7777 npm run dev:ui`.

**The payload on a PS5.**

```sh
npm run build:ui                                        # the embedded UI
tile/build.sh                                           # the embedded tile (optional)
make -C payload                                         # payload/build/openpsapp.elf
make -C payload deploy PS5_HOST=192.168.1.50            # to elfldr on port 9021
make -C payload prospero PS5_HOST=192.168.1.50          # or through Prospero Manager
```

`make -C payload ui` and `make -C payload tile` do the first two steps (`ui` runs `npm ci` at the root first).

**The website.** `npm run dev:web` serves it on `http://localhost:5174` and proxies `/api` to a cloud server. The server is closed source, so without one the site stops at its startup screen; the unit tests and type checks still run. See [web/README.md](web/README.md).

**The payload against a local server.** Build with `make -C payload DEV_CA=/path/to/ca.pem` to also trust a local test CA. The output goes to `payload/build-dev/` and its version ends in `+devca`. Never ship that build.

## Rules

### Code

- **No code comments** unless they are truly necessary, that is, they explain a non-obvious reason the code cannot show. Names and small functions should carry the meaning. This also applies to YAML; the only exception is the version comment after a pinned action SHA.
- **English** for identifiers, file names, commit messages and docs. User-facing text goes through i18next, in English, Portuguese (Brazil) and Spanish with the same keys; tests fail when a locale misses a key.
- **Formatting** is enforced: Prettier and ESLint for TypeScript, JSON, CSS and YAML (`web/src/components/ui/` is shadcn's code and is excluded), clang-format for C++. `payload/third_party/` is never reformatted.
- **Keep the layers.**
  - Payload: routes only handle HTTP, services hold the logic, formats parse console files, `platform/` isolates PS5 calls. Nothing may write console data; `test_readonly` checks it.
  - Website: follow the organization rules in [web/README.md](web/README.md).
  - Console UI: it must run in the PS5 browser; see the constraints in [ui/README.md](ui/README.md).
  - Logic that both `ui/` and `web/` need goes to `core/`; shapes that the server also uses go to `contract/`.
- **Tests** sit next to the code they cover (`*.test.ts`, `*.test.tsx`); payload tests live in `payload/tests/host/`. Bug fixes come with a test that fails without the fix.
- **No personal data.** Never commit real console data (`payload/tests/fixtures/` is ignored for that reason), and never put console ids, PSN ids, tokens, IP addresses of your network or personal names in code, tests, issues or pull requests. Use examples such as `192.168.1.50`.
- **No license headers.** The license is set per directory, as stated in the README.

### Dependencies

- **Pin exact versions** (no `^` or `~`).
- **Every new version must be at least 14 days old.** `.npmrc` sets `min-release-age=14`, so npm refuses anything newer, and Dependabot waits 14 days (`cooldown: default-days: 14`). Check a release date with `npm view <package> time --json`. The same rule applies to GitHub Actions (pinned by commit SHA with the version as a comment), the clang-format wheel, the .NET SDK and the SDK image.
- Dependabot security updates skip the cooldown. When the fix is younger than 14 days, the maintainer decides case by case.
- Dependabot ignores major updates of `typescript` (typescript-eslint needs the root on 6.0.x), `tailwindcss` (the console UI stays on Tailwind 3 for the PS5 browser) and `@types/node` (it follows the Node 24 runtime).
- **Overrides.** The root `package.json` forces `minimatch` to `11.0.3`. It fixes [GHSA-g3hv-75wg-m249](https://github.com/advisories/GHSA-g3hv-75wg-m249) (denial of service, fixed in 11.0.1), which comes in through `eslint`, `typescript-eslint` and `ts-morph` (a dependency of the `shadcn` CLI). Remove the override once all of them depend on minimatch 11.0.1 or later.
- **Two TypeScript versions.** typescript-eslint supports TypeScript below 6.1, so the root keeps TypeScript 6.0.3 for ESLint, while each workspace type-checks and builds with TypeScript 7.0.2. Unify them once typescript-eslint supports TypeScript 7.

### Commits

Use [Conventional Commits](https://www.conventionalcommits.org/): `type(scope): summary`, in the imperative and lowercase.

- Types: `feat`, `fix`, `refactor`, `perf`, `test`, `docs`, `build`, `ci`, `chore`.
- Scopes, when one part is involved: `payload`, `ui`, `web`, `core`, `contract`, `tile`, `tools`.
- Examples: `feat: monthly recap, milestones, stats and trophy timeline`, `refactor(ui): split by types, services, hooks and utils`.

## Pull requests

1. Fork the repository and create a branch from `main`.
2. Keep the change focused. Separate refactors from behaviour changes.
3. Run the [checks](#checks) for every part you touched.
4. Update the docs that your change affects: the README for anything users see, `SYNC.md` for anything that changes what the console sends, the area README, and the `Unreleased` section of [CHANGELOG.md](CHANGELOG.md).
5. Open the pull request and fill in the template. CI must pass before review.

Checklist:

- [ ] The checks for every touched part pass locally.
- [ ] Tests cover the change.
- [ ] No code comments unless truly necessary.
- [ ] New or updated dependencies are pinned exactly and at least 14 days old.
- [ ] User-facing text is translated into English, Portuguese (Brazil) and Spanish.
- [ ] Console UI changes work in the PS5 browser.
- [ ] Docs and `CHANGELOG.md` are updated.
- [ ] No console ids, PSN ids, tokens, IP addresses or personal names.
- [ ] Commits follow Conventional Commits.

## Releasing

Releases are built by [`.github/workflows/release.yml`](.github/workflows/release.yml) when a `v*` tag is pushed. The ELF takes its version from `git describe --tags`, so the tag is the version.

1. Move the `Unreleased` entries of `CHANGELOG.md` into a new version section, and commit that on `main`.
2. Tag the commit and push the tag:

   ```sh
   git tag -a vX.Y.Z -m "OpenPSApp vX.Y.Z"
   git push origin vX.Y.Z
   ```

3. The workflow then:
   - builds the console UI, the website, the tile and the ELF, and fails if `git describe` does not match the tag;
   - builds the ELF twice and fails if the two SHA-256 hashes differ;
   - packs `web/dist` into a reproducible `web.zip` (sorted entries, fixed timestamps, no extra attributes);
   - writes `SHA256SUMS` and a signed build provenance attestation for `openpsapp.elf`, `openpsapp-tile.pkg`, `licenses.txt` and `web.zip`;
   - creates the GitHub release with that version's `CHANGELOG.md` section as its notes, and those files;
   - prints `WEB_SHA256` in the job summary.
4. Deploy the website: the server downloads `web.zip` from the release and checks it against `WEB_SHA256`.

Anyone can check a release with `gh attestation verify openpsapp.elf --repo marcelorsdd/OpenPSApp`.
