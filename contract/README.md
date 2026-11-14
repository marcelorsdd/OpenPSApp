# Contract

`@openpsapp/contract` holds the TypeScript types for every message that crosses a boundary in OpenPSApp: between the console and the cloud server, and between either of them and the apps that read from them. It contains types only, no code.

## License

This folder is under the [MIT License](LICENSE), unlike the rest of the repository, which is GPL-3.0-or-later. That lets the closed-source cloud server, and any other software, use these types. The license applies to every file in `contract/`; the files carry no headers of their own.

## Files

| File         | Import                         | What it describes                                                                                                              |
| ------------ | ------------------------------ | ------------------------------------------------------------------------------------------------------------------------------ |
| `api.ts`     | `@openpsapp/contract/api`      | The read API that both the payload and the server serve: health, profile, games, trophy sets, trophies, earned trophies          |
| `pairing.ts` | `@openpsapp/contract/pairing`  | Linking a console: code request and answer, inspect and confirm from the website, token polling, and the device info the website lists |
| `sync.ts`    | `@openpsapp/contract/sync`     | What the console uploads: the summary (manifest) and the server's answer, trophy sets and trophies                               |

[`SYNC.md`](../SYNC.md) explains in plain words when each sync message is sent and what it contains.

## Who uses it

- The **payload** writes JSON that matches `api.ts`, `pairing.ts` and `sync.ts`. It is C++, so it does not import the types; tests and code review keep it in step.
- The **console UI** and the **website** import the types, through `@openpsapp/core` or directly.
- The **cloud server** keeps a byte-identical copy of these files, refreshed with `npm run contract:sync` in its own repository.

## Changing it

- A change here is an API change for the payload, the server and both apps. Keep fields backward compatible when you can: add optional fields rather than renaming or removing them, because consoles keep running older releases.
- Update `SYNC.md` when anything the console sends changes.
- Run `npm run typecheck -w @openpsapp/contract` and the type checks and tests of `core`, `ui` and `web`.
