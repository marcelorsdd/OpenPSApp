# Core

`@openpsapp/core` holds the pure TypeScript logic that the [console UI](../ui/README.md) and the [website](../web/README.md) share, so both show the same numbers, dates and trophy states. It has no DOM, network or React code, and every module has tests next to it.

| Module            | What it does                                                                                                          |
| ----------------- | --------------------------------------------------------------------------------------------------------------------- |
| `format.ts`       | Durations, dates, months, day names, "time ago" and decimals, formatted for a locale                                 |
| `dates.ts`        | Day and month keys in a time zone, month arithmetic                                                                  |
| `progress.ts`     | The monthly recap (trophies, game of the month, pace), completion, trophies grouped by day for the timeline          |
| `trophies.ts`     | Sorting and filtering trophy lists, recent trophies, and masking hidden trophies until they are earned or revealed   |
| `language.ts`     | The supported languages (`pt-BR`, `en`, `es`) and how a console or browser language maps to one of them              |
| `pairing-code.ts` | The pairing code alphabet and format: cleaning typed input, formatting as `XXXX-XXXX`, the QR link and its display URL |

Everything is exported from `src/index.ts`; import it as `@openpsapp/core`. Types of the data it works on come from [`@openpsapp/contract`](../contract/README.md).

`pairing-code.ts` cleans what people type, so it accepts spaces, dashes and lowercase letters. The server validates codes strictly on its side; keep `USER_CODE_ALPHABET` and `USER_CODE_LENGTH` in step with it.

## Commands

From the repository root:

```sh
npm test -w @openpsapp/core
npm run typecheck -w @openpsapp/core
```

## Rules

- Only functions, constants and types. Anything that needs the DOM, the network or React stays in `ui/` or `web/`.
- Code goes here when both `ui/` and `web/` need it; code that only one of them needs stays there.
- Every exported function has tests. Date and time functions take the time zone and the current time as arguments so tests are deterministic.
- `core/` must also run on the PS5 browser (Safari 13.1), because the console UI bundles it. See the [constraints](../ui/README.md#safari-131-constraints).
