import sharp from 'sharp';
import { fileURLToPath } from 'node:url';
import path from 'node:path';

const here = path.dirname(fileURLToPath(import.meta.url));
const out = (name) => path.join(here, 'sce_sys', name);

const trophy = `
  <path d="M-92 -120 H92 V-40 C92 22 50 64 0 64 C-50 64 -92 22 -92 -40 Z" fill="url(#gold)"/>
  <path d="M-92 -100 H-140 C-140 -40 -122 -6 -88 4" fill="none" stroke="url(#gold)" stroke-width="18" stroke-linecap="round"/>
  <path d="M92 -100 H140 C140 -40 122 -6 88 4" fill="none" stroke="url(#gold)" stroke-width="18" stroke-linecap="round"/>
  <rect x="-18" y="62" width="36" height="46" fill="url(#gold)"/>
  <rect x="-70" y="104" width="140" height="30" rx="8" fill="url(#gold)"/>
  <path d="M-38 -84 L-12 -58 L38 -104" fill="none" stroke="#0b1a33" stroke-width="16" stroke-linecap="round" stroke-linejoin="round"/>`;

const defs = `
  <defs>
    <linearGradient id="bg" x1="0" y1="0" x2="1" y2="1">
      <stop offset="0" stop-color="#0a3a8c"/>
      <stop offset="1" stop-color="#05122b"/>
    </linearGradient>
    <linearGradient id="gold" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0" stop-color="#ffe08a"/>
      <stop offset="1" stop-color="#d69a2d"/>
    </linearGradient>
  </defs>`;

const icon = `
<svg xmlns="http://www.w3.org/2000/svg" width="512" height="512" viewBox="0 0 512 512">
  ${defs}
  <rect width="512" height="512" fill="url(#bg)"/>
  <g transform="translate(256 220)">${trophy}</g>
  <text x="256" y="440" text-anchor="middle" font-family="Helvetica, Arial, sans-serif" font-size="64"
        font-weight="700" fill="#ffffff">OpenPSApp</text>
</svg>`;

const banner = `
<svg xmlns="http://www.w3.org/2000/svg" width="1920" height="1080" viewBox="0 0 1920 1080">
  ${defs}
  <rect width="1920" height="1080" fill="url(#bg)"/>
  <g transform="translate(1440 540) scale(2.4)" opacity="0.18">${trophy}</g>
</svg>`;

await sharp(Buffer.from(icon)).png().toFile(out('icon0.png'));
await sharp(Buffer.from(banner)).png().toFile(out('pic0.png'));
console.log('wrote sce_sys/icon0.png and sce_sys/pic0.png');
