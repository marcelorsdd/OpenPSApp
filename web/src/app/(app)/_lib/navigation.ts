import { paths } from '@/lib/links';
import type { RouteHandle } from '@/lib/route-handle';
import type { NavItem, Section, ShortcutKeys } from '../_types/navigation';

export const NAV_ITEMS: NavItem[] = [
  { section: 'profile', to: paths.profile },
  { section: 'trophies', to: paths.trophies },
  { section: 'account', to: paths.account },
];

export function sectionFor(pathname: string): Section | null {
  if (pathname === '/') return 'profile';
  if (pathname.startsWith('/trophies') || pathname.startsWith('/games/')) return 'trophies';
  if (pathname.startsWith('/account') || pathname.startsWith('/link')) return 'account';
  return null;
}

export function isFocusedRoute(matches: { handle: unknown }[]): boolean {
  return matches.some((match) => (match.handle as RouteHandle | undefined)?.focused === true);
}

export function shortcutTarget(key: string, keys: ShortcutKeys): string | null {
  const item = NAV_ITEMS.find((nav) => keys[nav.section].toLowerCase() === key.toLowerCase());
  return item ? item.to : null;
}
