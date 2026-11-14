import type { TrophyFilter, TrophySort } from '@openpsapp/core';
import { useState } from 'react';
import { useStoredState } from '@/hooks/use-stored-state';
import { parseTrophySort } from '../_lib/describe-game';

export function useTrophyList() {
  const [filter, setFilter] = useState<TrophyFilter>('all');
  const [storedSort, setSort] = useStoredState<TrophySort>('game.sort', 'default');
  const [showHidden, setShowHidden] = useState(false);
  const [revealed, setRevealed] = useState<ReadonlySet<number>>(() => new Set());
  return {
    filter,
    setFilter,
    sort: parseTrophySort(storedSort),
    setSort,
    showHidden,
    setShowHidden,
    revealed,
    reveal: (id: number) => setRevealed((current) => new Set(current).add(id)),
  };
}
