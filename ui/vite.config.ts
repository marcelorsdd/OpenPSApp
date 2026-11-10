import { fileURLToPath } from 'node:url';
import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';

const api = process.env.OPENPSAPP_API ?? 'http://localhost:7777';

export default defineConfig({
  plugins: [react()],
  resolve: {
    alias: { '@': fileURLToPath(new URL('./src', import.meta.url)) },
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
