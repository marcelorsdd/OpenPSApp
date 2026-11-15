import { isUserCode } from '@openpsapp/core';

const BARE_CODE = /^[A-Za-z2-9]{4}-?[A-Za-z2-9]{4}$/;
const LINK_PATHS = new Set(['/link', '/link/']);

function asCode(value: string): string | null {
  if (!BARE_CODE.test(value)) return null;
  const code = value.replace('-', '').toUpperCase();
  return isUserCode(code) ? code : null;
}

export function readScannedCode(raw: string, origin: string): string | null {
  const text = raw.trim();
  const bare = asCode(text);
  if (bare) return bare;
  let url: URL;
  try {
    url = new URL(text);
  } catch {
    return null;
  }
  if (url.origin !== origin || url.username || url.password || !LINK_PATHS.has(url.pathname)) return null;
  const codes = [...new URLSearchParams(url.hash.slice(1)).getAll('code'), ...url.searchParams.getAll('code')];
  return codes.length === 1 ? asCode(codes[0]) : null;
}
