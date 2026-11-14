import { useEffect, useMemo, useRef, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { useNavigate } from 'react-router';
import { useEarnedTrophies } from '@/hooks/use-earned-trophies';
import { useGames } from '@/hooks/use-games';
import { useTrophySets } from '@/hooks/use-trophy-sets';
import { shortcutTarget } from '../_lib/navigation';
import { searchGroups } from '../_lib/search';
import type { ShortcutKeys } from '../_types/navigation';

const SEQUENCE_MS = 1200;
const NO_ITEMS: never[] = [];

function isTyping(target: EventTarget | null): boolean {
  if (!(target instanceof HTMLElement)) return false;
  return target.isContentEditable || ['INPUT', 'TEXTAREA', 'SELECT'].includes(target.tagName);
}

export function useCommandSearch() {
  const { t } = useTranslation();
  const navigate = useNavigate();
  const [open, setOpen] = useState(false);
  const [query, setQuery] = useState('');
  const pendingG = useRef(0);
  const games = useGames();
  const sets = useTrophySets();
  const earned = useEarnedTrophies();
  const keys = useMemo<ShortcutKeys>(
    () => ({ profile: t('nav.keys.profile'), trophies: t('nav.keys.trophies'), account: t('nav.keys.account') }),
    [t],
  );

  useEffect(() => {
    const onKeyDown = (event: KeyboardEvent) => {
      if ((event.metaKey || event.ctrlKey) && event.key.toLowerCase() === 'k') {
        event.preventDefault();
        setOpen((value) => !value);
        return;
      }
      if (event.metaKey || event.ctrlKey || event.altKey || isTyping(event.target)) return;
      if (event.key === 'g') {
        pendingG.current = Date.now();
        return;
      }
      if (Date.now() - pendingG.current < SEQUENCE_MS) {
        const target = shortcutTarget(event.key, keys);
        pendingG.current = 0;
        if (target) {
          event.preventDefault();
          setOpen(false);
          void navigate(target);
        }
      }
    };
    window.addEventListener('keydown', onKeyDown);
    return () => window.removeEventListener('keydown', onKeyDown);
  }, [navigate, keys]);

  const changeOpen = (next: boolean) => {
    setOpen(next);
    if (!next) setQuery('');
  };

  return {
    open,
    query,
    keys,
    groups: searchGroups({ games: games.data ?? NO_ITEMS, sets: sets.data ?? NO_ITEMS, earned: earned.data ?? NO_ITEMS }, query),
    setOpen: changeOpen,
    setQuery,
    select: (to: string) => {
      changeOpen(false);
      void navigate(to);
    },
  };
}
