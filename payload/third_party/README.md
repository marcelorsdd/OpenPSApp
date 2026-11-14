# Third-party code

The payload vendors three C libraries. They are compiled from these folders by `payload/Makefile`, never reformatted (`.clang-format-ignore`, `.editorconfig`), and marked as vendored for GitHub (`.gitattributes`).

| Library | Version | License | Upstream | What is vendored |
| ------- | ------- | ------- | -------- | ---------------- |
| [SQLite](https://sqlite.org) | 3.54.1 | Public domain ([copyright](https://sqlite.org/copyright.html)) | https://sqlite.org/download.html | `sqlite/sqlite3.c`, `sqlite/sqlite3.h` from the amalgamation |
| [cJSON](https://github.com/DaveGamble/cJSON) | 1.7.19 | MIT (`cjson/LICENSE`) | https://github.com/DaveGamble/cJSON/releases | `cjson/cJSON.c`, `cjson/cJSON.h`, `cjson/LICENSE` |
| [Mbed TLS](https://github.com/Mbed-TLS/mbedtls) | 3.6.7 | Apache-2.0 (`mbedtls/LICENSE`) | https://github.com/Mbed-TLS/mbedtls/releases | `mbedtls/include/`, `mbedtls/library/`, `mbedtls/LICENSE` |

How each one is used:

- **SQLite** reads the console's `app.db`, opened read-only. It is built with `SQLITE_THREADSAFE=1`, `SQLITE_OMIT_LOAD_EXTENSION=1` and `SQLITE_DQS=0`.
- **cJSON** parses `config.json`, `cloud.json` and the server's answers.
- **Mbed TLS** is the payload's TLS 1.2 client. Its configuration is not in this folder: it is [`src/net/mbedtls_config.h`](../src/net/mbedtls_config.h), passed with `MBEDTLS_CONFIG_FILE`, which enables only what the client needs. Host builds also enable the TLS server (`OPENPSAPP_TLS_TEST_SERVER`) for the tests.

The console UI build lists all three in `licenses.txt` (`ui/vite.config.ts`), and the ELF serves that file.

## Updating

1. Download the new release from the upstream link above and check it against the checksum or signature its release page publishes.
2. Replace only the files listed under "What is vendored". Do not reformat them and do not edit them; if a change is needed, make it in OpenPSApp's code or, for Mbed TLS, in `src/net/mbedtls_config.h`.
   - SQLite: unzip `sqlite-amalgamation-<version>.zip` and copy `sqlite3.c` and `sqlite3.h`.
   - cJSON: copy `cJSON.c`, `cJSON.h` and `LICENSE` from the release tag.
   - Mbed TLS: unpack `mbedtls-<version>.tar.bz2`, then replace `include/` and `library/` wholesale and copy `LICENSE`.
3. Update the version in this file, in the [Third-party code](../README.md#third-party-code) section of `payload/README.md`, and, for Mbed TLS, in the `Mbed TLS <version>` entry of `ui/vite.config.ts`.
4. Run `make -C payload host-test` (it covers the HTTPS client against a local TLS server, `app.db` reading and JSON parsing) and `make -C payload`, then test linking and sync on a console.
5. Note the update in the `Unreleased` section of `CHANGELOG.md`.

Security releases of Mbed TLS and SQLite should be picked up quickly; watch their release pages or advisories.
