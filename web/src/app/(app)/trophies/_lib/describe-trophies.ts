import type { EarnedTrophy, Game, TrophySet } from '@openpsapp/contract/api';
import { GRADES, groupByDay, isComplete } from '@openpsapp/core';
import type { GradeTotal, SetCardView, SetSort, TimelineDay } from '../_types/trophies';

export const SET_SORTS: SetSort[] = ['recent', 'progress', 'name'];
export const TIMELINE_PREVIEW = 8;

export function parseSetSort(value: unknown): SetSort {
  return SET_SORTS.includes(value as SetSort) ? (value as SetSort) : 'recent';
}

export function setCards(sets: TrophySet[], games: Game[]): SetCardView[] {
  const byTitle = new Map(games.map((game) => [game.titleId, game]));
  return sets.map((set) => {
    const game = set.titleId ? byTitle.get(set.titleId) : undefined;
    return {
      id: set.id,
      name: game?.name ?? set.name,
      titleId: set.titleId,
      iconUrl: game?.iconUrl ?? set.iconUrl,
      heroUrl: game?.heroUrl ?? null,
      progress: set.progress,
      earned: set.earned,
      total: set.total,
      lastEarnedAt: set.lastEarnedAt,
      counts: set.counts,
    };
  });
}

export function visibleSets(sets: TrophySet[], sort: SetSort, incompleteOnly: boolean): TrophySet[] {
  const list = incompleteOnly ? sets.filter((set) => !isComplete(set)) : [...sets];
  if (sort === 'progress') return list.sort((a, b) => b.progress - a.progress || a.name.localeCompare(b.name));
  if (sort === 'name') return list.sort((a, b) => a.name.localeCompare(b.name));
  return list.sort((a, b) => (b.lastEarnedAt ?? '').localeCompare(a.lastEarnedAt ?? '') || a.name.localeCompare(b.name));
}

export function gradeTotals(sets: TrophySet[]): GradeTotal[] {
  return GRADES.map((grade) => ({
    grade,
    earned: sets.reduce((sum, set) => sum + set.counts[grade].earned, 0),
    total: sets.reduce((sum, set) => sum + set.counts[grade].total, 0),
  }));
}

export function timeline(earned: EarnedTrophy[], limit: number | null, timeZone?: string): TimelineDay[] {
  const days: TimelineDay[] = [];
  let left = limit ?? Number.POSITIVE_INFINITY;
  for (const group of groupByDay(earned, timeZone)) {
    if (left <= 0) break;
    days.push({ day: group.day, count: group.items.length, items: group.items.slice(0, left) });
    left -= group.items.length;
  }
  return days;
}
