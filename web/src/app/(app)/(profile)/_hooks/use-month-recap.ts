import type { EarnedTrophy, Game, TrophySet } from '@openpsapp/contract/api';
import { monthSummary, shiftMonth } from '@openpsapp/core';
import { useState } from 'react';
import { useNow } from '@/hooks/use-now';
import { monthRange, topGame } from '../_lib/describe-profile';

export function useMonthRecap(earned: EarnedTrophy[], games: Game[], sets: TrophySet[]) {
  const now = useNow();
  const range = monthRange(earned, now);
  const [chosen, setChosen] = useState<string | null>(null);
  const month = chosen ?? range.current;
  const summary = monthSummary(earned, month, now);
  return {
    month,
    total: summary.total,
    perWeek: summary.perWeek,
    top: topGame(summary.topSet, games, sets),
    canPrevious: month > range.first,
    canNext: month < range.current,
    previous: () => setChosen(shiftMonth(month, -1)),
    next: () => setChosen(shiftMonth(month, 1)),
  };
}
