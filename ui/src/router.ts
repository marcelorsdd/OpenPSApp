import { useEffect, useState } from 'react';

export type Route =
  | { name: 'profile' }
  | { name: 'trophies' }
  | { name: 'game'; titleId: string }
  | { name: 'set'; setId: string };

export function parseRoute(hash: string): Route {
  const parts = hash.replace(/^#\/?/, '').split('/').filter(Boolean).map(decodeURIComponent);
  if (parts[0] === 'trophies') return { name: 'trophies' };
  if (parts[0] === 'game' && parts[1]) return { name: 'game', titleId: parts[1] };
  if (parts[0] === 'set' && parts[1]) return { name: 'set', setId: parts[1] };
  return { name: 'profile' };
}

export const href = {
  profile: () => '#/',
  trophies: () => '#/trophies',
  game: (titleId: string) => `#/game/${encodeURIComponent(titleId)}`,
  set: (setId: string) => `#/set/${encodeURIComponent(setId)}`,
};

export function useRoute(): Route {
  const [route, setRoute] = useState(() => parseRoute(window.location.hash));
  useEffect(() => {
    const onChange = () => {
      setRoute(parseRoute(window.location.hash));
      window.scrollTo(0, 0);
    };
    window.addEventListener('hashchange', onChange);
    return () => window.removeEventListener('hashchange', onChange);
  }, []);
  return route;
}
