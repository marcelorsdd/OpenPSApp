# Tile

The Media-tab tile is a small PS5 package with no executable of its own. It only puts an OpenPSApp icon in the home screen's Media tab, and opening it sends the PS5 browser to `http://127.0.0.1:7777/`, where the payload serves the console UI. `make -C payload` embeds the package into the ELF, and the payload installs it on its first run with the console's package installer.

## What defines it

`sce_sys/param.json`:

| Field                     | Value                                    | Meaning                                                           |
| ------------------------- | ---------------------------------------- | ----------------------------------------------------------------- |
| `titleId`                 | `OPSA00001`                              | The tile's title id; the payload checks for it to know it is installed |
| `contentId`               | `IV9999-OPSA00001_00-OPENPSAPP0000001`   | The package's content id                                          |
| `deeplinkUri`             | `http://127.0.0.1:7777/`                 | What the tile opens. `embed.py` passes its port to the payload, which logs a mismatch with `port` in `config.json`. |
| `applicationCategoryType` | `65536`                                  | Places the tile in the Media tab                                  |

`sce_sys/icon0.png` is the icon and `sce_sys/pic0.png` the background.

The home screen keeps the category from a tile's first registration. To change the category or the deeplink, delete the tile on the console first; the payload reinstalls it on its next start.

## Building

```sh
tile/build.sh                          # Docker, the default
TILE_LOCAL_DOTNET=1 tile/build.sh      # a local .NET SDK 10, on Linux
make -C payload tile                   # the same as tile/build.sh
```

The script:

1. clones [LibProsperoPKG](https://github.com/SvenGDK/LibProsperoPKG) into `tile/.cache/` and checks out a pinned commit;
2. stages `sce_sys/param.json`, `icon0.png` and `pic0.png` in `tile/dist/stage/`;
3. runs `builder/OpenPSAppTile.csproj`, a small .NET program that calls LibProsperoPKG, either in the digest-pinned `mcr.microsoft.com/dotnet/sdk:10.0.402` image or with the local `dotnet`;
4. writes `tile/dist/openpsapp-tile.pkg`.

On macOS, use Docker: the PS5 package format needs SHA3-256, which .NET on macOS lacks. CI and releases use `TILE_LOCAL_DOTNET=1` with .NET SDK 10.0.402 on Ubuntu.

To update LibProsperoPKG or the .NET SDK, change `LIB_COMMIT` or `SDK_IMAGE` (tag and digest) in `build.sh`, and the `dotnet-version` in the workflows. Both must be at least 14 days old.

## Artwork

`art.mjs` draws the logo with [sharp](https://sharp.pixelplumbing.com/) and writes `sce_sys/icon0.png`, `sce_sys/pic0.png` and the website's icons in `web/public/`:

```sh
npm run art -w @openpsapp/tile
```

Commit the regenerated images.
