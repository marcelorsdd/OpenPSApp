# Console UI

`@openpsapp/ui` is the app people see on the PS5: React 19, Vite and Tailwind CSS 3. `make -C payload` embeds its build (`ui/dist`) into the ELF, which serves it at `/`. The Media-tab tile opens it in the PS5 browser at `http://127.0.0.1:7777/`, and any browser on the same network can open it at `http://<console-ip>:7777`.

It is console-only: it talks to the payload's `/api/v1` on the same origin and nothing else. Clerk and the website code are not part of it; CI fails if `clerk` appears anywhere in `ui/dist`. The website lives in [`web/`](../web/README.md).

## Commands

From the repository root, after `npm ci`:

| Command                              | What it does                                                                                    |
| ------------------------------------ | ----------------------------------------------------------------------------------------------- |
| `npm run dev:ui`                     | Vite dev server; proxies `/api` to `http://localhost:7777` (`OPENPSAPP_API` to change it)       |
| `npm run build:ui`                   | Type-checks and builds `ui/dist`, including `licenses.txt`                                      |
| `npm test -w @openpsapp/ui`          | Vitest: unit tests in Node, `*.test.tsx` in jsdom                                               |
| `npm run typecheck -w @openpsapp/ui` | TypeScript for the app and `vite.config.ts`                                                     |

To work without a console, run `make -C payload host-run` in another terminal; it serves the API over a fake console. To use a real console: `OPENPSAPP_API=http://192.168.1.50:7777 npm run dev:ui`. Linking, unlinking and **Sync now** only work from the console itself, so they answer `console_only` from the dev server.

## Safari 13.1 constraints

The PS5 browser is roughly Safari 13.1. Code that works in a current browser can fail silently there, and there is no automated check for it: test UI changes on a console.

- **Build target.** `vite.config.ts` sets `build.target` and `build.cssTarget` to `safari13.1`. Syntax is lowered, but no runtime APIs are polyfilled.
- **JavaScript APIs to avoid**, among others: `Array.prototype.at` and `findLast`, `String.prototype.at`, `Object.hasOwn`, `structuredClone`, `crypto.randomUUID` (all Safari 15.4), `WeakRef` (14.1), `Promise.any` and `Intl.RelativeTimeFormat` (14), and `BarcodeDetector`. Check a feature's Safari version before using it.
- **Tailwind CSS 3, not 4.** Tailwind 4 needs Safari 16.4.
- **CSS to avoid:** `gap` on flex containers (Safari 14.1; use margins, as the components do with `ml-3` or `-mx-1` plus `mx-1`), `aspect-ratio` (15), and the `inset` shorthand (14.1), which Tailwind's `inset-*` utilities produce. `gap` on grids is fine. `:focus-visible` (15.4) is ignored, so do not rely on it alone.
- **Storage can fail.** Read and write `localStorage` through `useStoredState`, which falls back to defaults.
- **Everything is embedded.** No CDNs, web fonts or external requests; the font stack uses system fonts. `assetsInlineLimit: 0` keeps every asset as a file that the ELF serves from `/assets/`.
- **Hash routes.** The payload serves only `/`, `/:file` and `/assets/:file`, so routes live in the hash: `#/`, `#/trophies`, `#/game/:titleId`, `#/set/:setId`, `#/account`.
- **A TV screen.** The root font size is 18px and focus outlines are large, because people navigate with a controller from a sofa.

## Layout

```
ui/src/
├── types/        API types (re-exported from @openpsapp/contract), cloud status and routes
├── services/     api.ts: typed endpoints and the only fetch calls; POSTs send X-OpenPSApp: 1
├── hooks/        useApi, useRoute, useStoredState, useLanguageChoice, useDate
├── utils/        router, error codes to i18n keys, QR matrix (tests next to them)
├── components/   ui/ (generic), trophies/, profile/, game/
├── pages/        one component per route: Profile, Trophies, Set, Game, Account
└── i18n/         i18next setup and locales
```

Imports use the `@/` alias for `ui/src`. Each endpoint in `services/api.ts` carries its response type, so `useApi(endpoints.profile)` is typed without a generic. Formatting, dates, progress, trophy sorting and masking, languages and pairing helpers come from [`@openpsapp/core`](../core/README.md); API shapes come from [`@openpsapp/contract`](../contract/README.md).

## Translations

`src/i18n/locales/` has `en.json`, `es.json` and `pt-BR.json` with the same keys; a test fails if a locale misses one. By default the UI follows the language in `/api/v1/health` (the console's language, or `language` in `config.json`). The picker in the header overrides it and is saved in the browser. To add a language, add a locale file, register it in `src/i18n/index.ts`, add it to `LANGUAGES` in `core/src/language.ts`, and do the same in `web/`.
