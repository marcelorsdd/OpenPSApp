import { fileURLToPath } from 'node:url';
import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';
import licenses from './plugins/licenses.ts';

const api = process.env.OPENPSAPP_API ?? 'http://localhost:7777';
const local = (file: string) => fileURLToPath(new URL(file, import.meta.url));

const header = `OpenPSApp is free software under the GNU General Public License v3.0 or later.
Source code: https://github.com/marcelorsdd/OpenPSApp
The payload is built with ps5-payload-sdk (GPL-3.0): https://github.com/ps5-payload-dev/sdk

It includes the following third-party software.`;

const extras = [
  { name: 'tailwindcss (generated CSS)', license: 'MIT', file: local('./node_modules/tailwindcss/LICENSE') },
  { name: 'cJSON', license: 'MIT', file: local('../payload/third_party/cjson/LICENSE') },
  { name: 'SQLite', license: 'Public Domain', text: 'SQLite is in the public domain: https://sqlite.org/copyright.html' },
];

export default defineConfig({
  plugins: [react(), licenses(header, extras)],
  resolve: {
    alias: { '@': local('./src') },
  },
  server: {
    proxy: { '/api': api },
  },
  build: {
    target: 'safari13.1',
    cssTarget: 'safari13.1',
    assetsInlineLimit: 0,
  },
});
