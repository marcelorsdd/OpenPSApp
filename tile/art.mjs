import sharp from 'sharp';
import { fileURLToPath } from 'node:url';
import path from 'node:path';

const here = path.dirname(fileURLToPath(import.meta.url));
const out = (name) => path.join(here, 'sce_sys', name);

const BACKGROUND = '#101a33';
const RING = '#b4c8e0';
const GOLD = '#e3b341';

const mark = (ring, cup) => `
  <circle cx="50" cy="50" r="42" fill="none" stroke="${ring}" stroke-width="6" stroke-linecap="round"
          stroke-dasharray="205 264" transform="rotate(-4.75 50 50)"/>
  <path d="M34 30 H66 V43 C66 54 59 61 50 61 C41 61 34 54 34 43 Z" fill="${cup}"/>
  <path d="M34 34 H28 C28 42 31 47 36 49" fill="none" stroke="${cup}" stroke-width="4" stroke-linecap="round"/>
  <path d="M66 34 H72 C72 42 69 47 64 49" fill="none" stroke="${cup}" stroke-width="4" stroke-linecap="round"/>
  <rect x="47" y="60" width="6" height="8" fill="${cup}"/>
  <rect x="39" y="67" width="22" height="6" rx="2" fill="${cup}"/>`;

const icon = `
<svg xmlns="http://www.w3.org/2000/svg" width="512" height="512" viewBox="0 0 512 512">
  <rect width="512" height="512" fill="${BACKGROUND}"/>
  <g transform="translate(76 76) scale(3.6)">${mark(RING, GOLD)}</g>
</svg>`;

const banner = `
<svg xmlns="http://www.w3.org/2000/svg" width="1920" height="1080" viewBox="0 0 1920 1080">
  <rect width="1920" height="1080" fill="${BACKGROUND}"/>
  <g transform="translate(1160 220) scale(6.4)" opacity="0.16">${mark(RING, RING)}</g>
</svg>`;

await sharp(Buffer.from(icon)).png().toFile(out('icon0.png'));
await sharp(Buffer.from(banner)).png().toFile(out('pic0.png'));
console.log('wrote sce_sys/icon0.png and sce_sys/pic0.png');
