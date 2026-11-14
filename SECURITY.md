# Security policy

## Reporting a vulnerability

Please report vulnerabilities privately through GitHub's private vulnerability reporting:

**https://github.com/marcelorsdd/OpenPSApp/security/advisories/new**

If you cannot use GitHub, e-mail [marcelorsdd@gmail.com](mailto:marcelorsdd@gmail.com) instead.

Do not open a public issue, pull request or discussion for a vulnerability. Include what you found, how to reproduce it, the OpenPSApp version (the `version` field of `http://<console-ip>:7777/api/v1/health`, or the commit), and the impact you expect. Leave out real console ids, tokens and IP addresses.

OpenPSApp is maintained by one person in their spare time. You will get a first answer within **3 business days**, and we will agree on a fix and a disclosure date in the advisory. Credit is given in the advisory and the release notes unless you prefer otherwise.

## Supported versions

Only the [latest release](https://github.com/marcelorsdd/OpenPSApp/releases/latest) gets security fixes. The website and the cloud API are always the current version.

## Scope

In scope:

- **The payload's LAN API**: everything the ELF serves on port 7777 (`/api/v1/...` and the embedded files).
- **The console UI** in `ui/`.
- **The website** in `web/`.
- **The cloud API**, including pairing, sync and the read API. Its source is private, but reports about its behaviour are welcome here.
- The release and CI workflows in `.github/`.

Out of scope:

- Anything that requires code execution on the jailbroken console. Such code can already read every file OpenPSApp keeps, including the token.
- The jailbreak, payload loaders and ps5-payload-sdk. Report those to their projects.
- Debug endpoints, which are off by default and documented as exposing console files to the network when `"debug": true`.
- Findings that only show that the LAN API is readable without a password. That is by design and documented in the README.
- Vulnerabilities in dependencies with no effect on OpenPSApp. Report them upstream; tell us if OpenPSApp is affected.

## Threat model

What OpenPSApp protects: the user's synced gaming history (profile name, games, playtime, trophies), the console's device token, and the user's account on the website.

| Who                                       | What we assume                                                                                       |
| ----------------------------------------- | ---------------------------------------------------------------------------------------------------- |
| Other devices on the LAN                  | They can read the LAN API and hide games, but must not link, unlink or sync the console.             |
| Websites open in a browser on the LAN     | They must not change anything on the console (CSRF) or reach the cloud actions (DNS rebinding).      |
| The network between console and server    | It must not read or change the traffic, or impersonate the server.                                   |
| Someone with a pairing link               | They must not get a console linked to their account without the owner seeing the code on the TV.     |
| Someone with a copy of the database       | They must not be able to use the device tokens.                                                      |
| The server operator                       | **Can read the synced data.** It is not end-to-end encrypted.                                        |

## Existing protections

- **Loopback gating.** Linking, unlinking, cancelling a code and **Sync now** only work when the request comes from `127.0.0.1` with a `Host` of `127.0.0.1:<port>` or `localhost:<port>`. Other clients never see the pairing code.
- **`Host` allowlist against DNS rebinding.** The payload answers only requests whose `Host` is `localhost` or an IP address; any host name gets `403 bad_host`, so a website cannot rebind its own name to the console and read the API.
- **A custom header against CSRF.** Every `POST` route requires `X-OpenPSApp: 1`. The payload sends no CORS headers, so a page on another origin cannot add that header.
- **Request limits.** 16 KiB of headers, 64 KiB of body, a 10 s deadline per request and 32 concurrent connections.
- **Pinned roots.** The payload's own HTTPS client (mbedTLS) refuses plain HTTP, checks the certificate and host name, and trusts only ISRG Root X1 and X2. Responses are capped at 8 MiB.
- **Hashed tokens.** Device tokens are random 256-bit secrets; the server stores only their SHA-256 hash. A device token can only send data and is revocable from the website or the console. Pairing codes expire after 10 minutes and work once.
- **CSP.** The server sends a Content Security Policy and other security headers with the website.
- **Read-only console access.** The payload opens console data read-only and writes only to `/data/openpsapp`; a test checks that no route writes console data.
- **Fuzzing.** The payload's parsers (HTTP requests and responses, URLs, trophy and console files, `config.json`, `cloud.json`) have libFuzzer targets run with ASan and UBSan (`make -C payload fuzz`, weekly in CI).
- **Supply chain.** Dependencies are pinned and at least 14 days old, GitHub Actions are pinned by commit SHA, and releases carry SHA-256 sums and a signed build provenance attestation.

[`SYNC.md`](SYNC.md) documents exactly what the console sends and how linking works. [ARCHITECTURE.md](ARCHITECTURE.md) shows the trust boundaries.
