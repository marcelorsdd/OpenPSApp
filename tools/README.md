# Tools

Build helpers shared by the Vite apps. The payload's own build helper is [`payload/tools/embed.py`](../payload/README.md#source-layout).

## `licenses.ts`

A Vite plugin that writes `licenses.txt` into the build output. Both [`ui/vite.config.ts`](../ui/vite.config.ts) and [`web/vite.config.ts`](../web/vite.config.ts) use it.

```ts
licenses(header, extras, unbundled);
```

- It collects every npm package whose code ends up in the bundle and adds the package's license file under a `name version (license)` title, sorted by title.
- `header` is the text at the top of the file.
- `extras` adds notices for code that is not an npm package in the bundle: the payload's vendored C libraries, generated CSS, code compiled into WebAssembly. Each entry takes a license file (`file`) or a text (`text`).
- `unbundled` gives the license text for packages that ship without a license file.
- The build fails if a bundled package has no license text, so a missing notice never ships.

Helpers for the configs:

- `packageFile(from, name, file)` finds a file of an installed package, such as its `LICENSE`.
- `mit(copyright)` and `apache2(copyright)` build a full license text. `apache2` reads `licenses/apache-2.0.txt`.

The console's `licenses.txt` is embedded in the ELF and served at `/licenses.txt`; the website's is served by the cloud server. Releases attach the console's file.

Run its type check with `npm run typecheck -w @openpsapp/ui` or `-w @openpsapp/web`; both include `tools/licenses.ts` through their `tsconfig.node.json`.
