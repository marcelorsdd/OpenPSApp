import type { Route } from '@/types/route';

const PROFILE: Route = { name: 'profile' };

function decodeParts(path: string): string[] | null {
  try {
    return path.split('/').filter(Boolean).map(decodeURIComponent);
  } catch {
    return null;
  }
}

export function parseRoute(hash: string): Route {
  const parts = decodeParts(hash.replace(/^#\/?/, '').split('?')[0]);
  if (!parts) return PROFILE;
  if (parts[0] === 'trophies') return { name: 'trophies' };
  if (parts[0] === 'game' && parts[1]) return { name: 'game', titleId: parts[1] };
  if (parts[0] === 'set' && parts[1]) return { name: 'set', setId: parts[1] };
  if (parts[0] === 'account') return { name: 'account' };
  return PROFILE;
}

export const href = {
  profile: () => '#/',
  trophies: () => '#/trophies',
  game: (titleId: string) => `#/game/${encodeURIComponent(titleId)}`,
  set: (setId: string) => `#/set/${encodeURIComponent(setId)}`,
  account: () => '#/account',
};
