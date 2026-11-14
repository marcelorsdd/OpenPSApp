# Website

`@openpsapp/web` is the OpenPSApp website: sign in, see the synced profile, trophies and games from any browser, manage the linked PS5 and the account, and link a console by typing its code or scanning its QR code. It is a single-page app built with React 19, Vite, Tailwind CSS 4, shadcn (`base-nova` on Base UI), React Router, TanStack Query, react-hook-form with zod, i18next and Clerk.

The cloud server serves it. Each release attaches the build as `web.zip`, and the server downloads that file and checks it against `WEB_SHA256` (see [Releasing](../CONTRIBUTING.md#releasing)). The server answers the browser-history routes (`/`, `/trophies`, `/games/:titleId`, `/account`, `/link?code=`) with `index.html`.

## Commands

From the repository root, after `npm ci`:

| Command                                | What it does                                                                                      |
| -------------------------------------- | ------------------------------------------------------------------------------------------------- |
| `npm run dev:web`                      | Dev server on `http://localhost:5174`, proxying `/api` to `http://127.0.0.1:3000`                  |
| `npm run build:web`                    | Type-checks and builds `web/dist`, with `licenses.txt`, the web manifest and icons                |
| `npm test -w @openpsapp/web`           | Vitest: `*.test.ts` in Node, `*.test.tsx` in jsdom                                                |
| `npm run typecheck -w @openpsapp/web`  | TypeScript for the app and `vite.config.ts`                                                       |

`OPENPSAPP_API=http://127.0.0.1:3020 npm run dev:web` points the proxy at another server.

## Running it against a server

The site reads the Clerk publishable key from `GET /api/v1/health` before it mounts Clerk, so one build works with any server. Without a server, it stops at the startup screen.

The server is closed source, so outside contributors cannot run one locally. You can still work on most of the site through its unit tests and type checks, since screens get their data through hooks and pure functions in `_lib`. Maintainers run the server locally with `PUBLIC_URL=http://localhost:5174`, so the pairing link and QR code on the console point at the dev site. The API shapes are in [`contract/`](../contract/README.md).

## Organization

`web/src` follows these rules:

- `app/<route>/page.tsx` is the entry of a route and `app/router.tsx` wires the routes up. Route groups in parentheses, such as `(app)` and `(site)`, group routes without adding to the URL.
- Next to a route live:
  - `_components`: components that get everything through props and `on*` callbacks;
  - `_hooks`: hooks only;
  - `_lib`: functions and constants only, with tests;
  - `_schemas`: zod schemas;
  - `_types`: view types.
- A file lives in the lowest route that contains every screen using it. `app/(app)/_lib` serves all signed-in pages; `app/(app)/(profile)` is `/`. A route never imports from a sibling route or another group.
- `src/hooks/` and `src/lib/` hold only what two or more routes use. What both `ui/` and `web/` need goes to [`core/`](../core/README.md).
- `src/components/` holds shared components; `src/components/ui/` is shadcn's.
- Async state is TanStack Query; forms are react-hook-form with a zod schema; UI text goes through i18next.

## Design tokens

- Every design value lives in `src/app/globals.css`: colors, radii, shadows, fonts, spacing and container sizes, as CSS variables mapped into Tailwind's theme with `@theme inline`.
- Screens use those tokens through Tailwind classes (`bg-card`, `text-muted-foreground`, `max-w-form`, `size-cover-md`). Never write hex colors or arbitrary values such as `top-[42%]` in a route; add a token to `globals.css` instead.
- `src/components/ui/` is shadcn's code, installed with the CLI. Its design values come from the same tokens. It is excluded from ESLint and Prettier so the CLI can update it.

## Adding a shadcn component

From `web/`:

```sh
npx shadcn add <component>
```

`components.json` sets the `base-nova` style, the `@/` aliases and `cn` as the class helper. Commit the generated file under `src/components/ui/` as is, and check that the new dependencies it adds are pinned and at least 14 days old.

## Translations

`src/i18n/locales/` has `en.json`, `es.json` and `pt-BR.json` with the same keys; a test fails if a locale misses one. The site follows the browser language; **Account → Preferences** overrides it and saves the choice in the browser.

## Licenses

`npm run build:web` writes `web/dist/licenses.txt` with the license of every bundled package (`tools/licenses.ts`), plus Tailwind CSS, tw-animate-css, shadcn and zxing-cpp, which is compiled into the zxing-wasm QR reader. The server serves it at `/licenses.txt`. The zxing-wasm reader loads only when the browser has no `BarcodeDetector`.
