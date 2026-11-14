import { fileURLToPath } from 'node:url';
import tailwindcss from '@tailwindcss/vite';
import react from '@vitejs/plugin-react';
import { defineConfig } from 'vitest/config';
import licenses, { apache2, mit, packageFile } from '../tools/licenses.ts';

const api = process.env.OPENPSAPP_API ?? 'http://127.0.0.1:3000';
const local = (file: string) => fileURLToPath(new URL(file, import.meta.url));
const here = local('.');

const header = `The OpenPSApp website is free software under the GNU General Public License v3.0 or later.
Source code: https://github.com/marcelorsdd/OpenPSApp

It includes the following third-party software.`;

const extras = [
  { name: 'tailwindcss (generated CSS)', license: 'MIT', file: packageFile(here, 'tailwindcss', 'LICENSE') },
  { name: 'tw-animate-css (generated CSS)', license: 'MIT', file: packageFile(here, 'tw-animate-css', 'LICENSE') },
  { name: 'shadcn (generated CSS and components)', license: 'MIT', file: packageFile(here, 'shadcn', 'LICENSE.md') },
  {
    name: 'zxing-cpp (compiled into the zxing-wasm reader)',
    license: 'Apache-2.0',
    text: apache2('Copyright 2016 Nu-book Inc. and the ZXing authors\nhttps://github.com/zxing-cpp/zxing-cpp'),
  },
];

const unbundled = {
  'react-remove-scroll-bar': mit('Copyright (c) 2017 Anton Korzunov'),
};

export default defineConfig({
  plugins: [react(), tailwindcss(), licenses(header, extras, unbundled)],
  resolve: {
    alias: { '@': local('./src') },
  },
  server: {
    port: 5174,
    strictPort: true,
    proxy: { '/api': api },
  },
  preview: {
    port: 5174,
    strictPort: true,
    proxy: { '/api': api },
  },
  test: {
    projects: [
      { extends: true, test: { name: 'unit', environment: 'node', include: ['src/**/*.test.ts'] } },
      { extends: true, test: { name: 'dom', environment: 'jsdom', include: ['src/**/*.test.tsx'] } },
    ],
  },
});
