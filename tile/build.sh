#!/usr/bin/env bash

set -euo pipefail
cd "$(dirname "$0")"

LIB_REPO=https://github.com/SvenGDK/LibProsperoPKG.git
LIB_COMMIT=748eabf1b7d17819528cabf367d8e27109d8fce3

SDK_IMAGE=mcr.microsoft.com/dotnet/sdk:10.0

if [ ! -d .cache/LibProsperoPKG ]; then
  git clone -q "$LIB_REPO" .cache/LibProsperoPKG
fi
git -C .cache/LibProsperoPKG checkout -q "$LIB_COMMIT"

rm -rf dist && mkdir -p dist/stage/sce_sys dist/out
cp sce_sys/param.json sce_sys/icon0.png sce_sys/pic0.png dist/stage/sce_sys/

rm -rf builder/bin builder/obj .cache/LibProsperoPKG/src/LibProsperoPkg/bin .cache/LibProsperoPKG/src/LibProsperoPkg/obj
if [ "${TILE_LOCAL_DOTNET:-0}" = 1 ]; then
  dotnet run --project builder -c Release -- dist/stage dist/out
else
  docker run --rm -v "$PWD":/work -w /work -e DOTNET_CLI_TELEMETRY_OPTOUT=1 -e DOTNET_NOLOGO=1 "$SDK_IMAGE" \
    dotnet run --project builder -c Release -- dist/stage dist/out
fi

pkg=$(find dist/out -name '*.pkg' | head -1)
if [ -z "$pkg" ]; then
  echo "build.sh: builder produced no .pkg" >&2
  exit 1
fi
mv "$pkg" dist/playtrack-tile.pkg
echo "built dist/playtrack-tile.pkg ($(wc -c < dist/playtrack-tile.pkg | tr -d ' ') bytes)"
