# Console data sources

These sources were confirmed on a real console on 2026-11-08 by running `GET /api/v1/debug/sources` on the payload and inspecting the pulled files. The firmware was not recorded. Every source listed here is read-only for PlayTrack.

## Playtime: `/system_data/priv/mms/app.db` (SQLite)

- Each user has a table `tbl_iconinfo_<user id, zero-padded decimal>`, for example `tbl_iconinfo_0515316669` for user `0x1eb71bbd`. Other tables that carry the same suffix: `tbl_addon_`, `tbl_concepticoninfo_`, `tbl_group_`, `tbl_info_`, `tbl_playlist_`.
- `playedTimeOnConsole` holds total seconds. It is `NULL` for titles that were installed but never played. Validated: Marvel's Wolverine read 51494 s, which matched what the console itself shows (14h18).
- `lastPlayedDateOnConsole` (`YYYY-MM-DD HH:MM:SS.fff`) moves only when the title is played. PlayTrack uses it as `lastPlayedAt`.
- Do not use `lastAccessTime` for this. It also moves when a tile is merely selected on the home screen. On the test console, two titles that were never played had a `lastAccessTime` but no `lastPlayedDateOnConsole`.
- Columns that are present but not used yet:
    - `gltTotalPlayedTime`, `gltPlayCount` and `gltlastPlayedDate` are empty on an offline console.
    - `conceptId`, `installedDate` and `titleName` are populated.
- `tbl_contentinfo` (33 rows) supplies `titleName`, `categoryType` and `is3rdPartyVideoApp`. Media apps such as YouTube are filtered out through these flags. It also supplies `metaDataPath` (`/user/app/<titleId>/sce_sys`) and `icon0Info`.
- There is no `tbl_appbrowse` on this firmware. The code treats that table as optional.
- Game icons are at `/user/appmeta/<titleId>/icon0.png`.

## Trophies

| What               | Path                                                                  | Format                  |
| ------------------ | --------------------------------------------------------------------- | ----------------------- |
| Trophy set catalog | `/user/trophy2/nobackup/conf/<NPWR>/TROPHY.UCP`                       | UCP container, 10–15 MB |
| Per-user progress  | `/user/home/<user id, hex>/trophy2/nobackup/data/<NPWR>/TRPTITLE.DAT` | T2PD, about 15 KB       |
| Game → set binding | `/user/app/<titleId>/sce_sys/trophy2/npbind.dat`                      | 532 B, TLV              |

The trophy home directory uses the **hex** user id (`1eb71bbd`). The `app.db` tables use the **decimal** id.

### UCP (verified)

The header is big-endian:

| Offset | Field              |
| ------ | ------------------ |
| `0x00` | magic `0xb228c60a` |
| `0x04` | `1`                |
| `0x08` | total size (u64)   |
| `0x10` | entry count        |
| `0x14` | entry size `64`    |

The entry table starts at `0x60`. Each 64-byte entry holds `name[32]`, then `offset` (u64), then `size` (u64).

The UCP contains these entries:

- **`tropconf.json`** has the keys `trophyNpCommId`, `defaultLanguage` and `languages[]`. Its `trophies[]` array holds objects of the form `{ "id": "0000", "hidden": false, "grade": "P", "hasReward": false }`, where `grade` is one of `P`, `G`, `S` or `B`.
- **`tropmeta_<lang>.json`** holds `metadata.titleMetadata.name` and `metadata.trophyMetadata[]`, each entry shaped `{ id, name, detail }`. The test sets ship 14–22 languages, including `pt-BR`.
- **Images:**
    - `icon0_<lang>.png` is the set icon.
    - `tropNNNN.png` is one image per trophy.

### npbind.dat

- The magic is `0xd294a018`.
- TLV records start at `0x80`. The record with tag `0x0010` and length `0x000c` holds the NP communication id, for example `NPWR57054_00`.
- The test console showed these bindings:
    - `PPSA03671` (Marvel's Wolverine) → `NPWR57054_00`
    - `PPSA02530` (PRAGMATA) → `NPWR53259_00`
    - `PPSA11386` (007 First Light) → `NPWR38314_00`. This set has no conf or data directory yet, because the game was never launched.

### TRPTITLE.DAT (verified)

The file is a T2PD paged stream. Its layout was verified against the progress files of two real trophy sets.

1. A 64-byte header holds `"T2PD"`, then `0x10000`, then `1056`, with the page count at `0x20`.
2. Pages follow. Each page is 1024 data bytes plus a SHA-256 of that data, and every real page passed the check.
3. The concatenated page data forms a `"T2TD"` stream. It starts with a directory of 9 entries, each 32 bytes: tag, payload, version, repeat, offset.

The trophy states are under tag `0x800`, one 96-byte record per trophy:

| Offset  | Field          |
| ------- | -------------- |
| `+0x10` | trophy id      |
| `+0x14` | flags          |
| `+0x20` | earned time    |
| `+0x30` | secondary time |

Times are microseconds since 0001-01-01.

How to read the flags:

- **Earned:** flags `0x11` with both times set. On the test console, Marvel's Wolverine has 21 of 42 earned.
- **Locked, with progress:** flags `0x10` with only the secondary time set.

Grades and the hidden flag come from `tropconf.json`, so the `0x500` definition records are not needed.

The progress percentage weights trophies by points (P 180, G 90, S 30, B 15). These weights have not been checked yet against the percentage the console shows.

## Payload runtime notes

- **Worker threads have small stacks.** A 64 KiB stack buffer overflowed one and froze the whole process. Once the fault handler had wedged the process, even SIGKILL did not end it. The build now fails on any stack frame larger than 8 KiB, and threads are started through `Spawn()`, which sets a 256 KiB stack and an alternate signal stack.
- **elfldr names every payload `payload.elf`.** PlayTrack renames itself to `playtrack.elf` with `thr_set_name`.
- **Prospero Manager has a usable HTTP API.** `make prospero` uploads the payload with `POST /api/payloads/upload?name=` (raw body) and starts it with `POST /api/payloads/launch`. Prospero also exposes `/api/processes`, `/api/processes/kill` and `/api/ftp/{list,text,download}` for diagnosis.
- **The PKG installer runs in another mount namespace.** A file the payload writes to `/data/x` is visible to `sceAppInstUtilAppInstallPkg` as `/user/data/x`. If `AppInstallPkg` fails, PlayTrack falls back to `InstallByPackage` with a `file://` URI. `PkgPlayGoInfo` is about 10 KiB, so it is allocated on the heap.
- **The notification syscall can stall**, so toasts are sent from their own thread.
- **`categoryType & 0x10000` in `tbl_contentinfo` marks Media-tab apps** (YouTube, homebrew tiles). These titles are left out of the games list.
- **`SIGPIPE` and `SIGHUP` are ignored.** A client that closes its socket, or the loader closing the connection it started the payload from, must not kill the process.
- **Icon URLs are `/api/v1/icons/<id>`.** The id is a hash of the file path, the byte range and the file's size and mtime, so no image is read until a browser requests it, and a changed file gets a new URL. That is why the URL can be cached forever.
- **Parsed trophy sets are cached** until the size or mtime of `TROPHY.UCP` or `TRPTITLE.DAT` changes. The library (playtime, trophies and the links between them) is rebuilt at most every 30 s, and immediately when the foreground user changes.
- **Only failed requests are logged, and only errors are fsync'd.** Successful icon requests would otherwise add one disk sync each.
