import type { Trophy } from '@openpsapp/contract/api';
import { filterTrophies, recentlyEarned, sortTrophies, type TrophyFilter, type TrophySort, trophyView } from '@openpsapp/core';
import type { TrophyCounts, TrophyListOptions, TrophyRowView } from '../_types/game';

export const TROPHY_FILTERS: TrophyFilter[] = ['all', 'earned', 'locked'];
export const TROPHY_SORTS: TrophySort[] = ['default', 'earned', 'grade'];
export const RECENT_TROPHIES = 4;

export function parseTrophySort(value: unknown): TrophySort {
  return TROPHY_SORTS.includes(value as TrophySort) ? (value as TrophySort) : 'default';
}

export function trophyCounts(trophies: Trophy[]): TrophyCounts {
  const earned = trophies.filter((t) => t.earned).length;
  return {
    all: trophies.length,
    earned,
    locked: trophies.length - earned,
    hidden: trophies.filter((t) => t.hidden && !t.earned).length,
  };
}

export function trophyRows(trophies: Trophy[], options: TrophyListOptions): TrophyRowView[] {
  return sortTrophies(filterTrophies(trophies, options.filter), options.sort).map((trophy) => {
    const view = trophyView(trophy, options.showHidden || options.revealed.has(trophy.id));
    return {
      id: trophy.id,
      grade: trophy.grade,
      name: view.name,
      description: view.description,
      iconUrl: view.iconUrl,
      earned: trophy.earned,
      earnedAt: trophy.earnedAt,
      masked: view.masked,
    };
  });
}

export function recentTrophies(trophies: Trophy[]): Trophy[] {
  return recentlyEarned(trophies, RECENT_TROPHIES);
}

export function backTarget(state: unknown, fallback: string): string {
  const from = (state as { from?: unknown } | null)?.from;
  return typeof from === 'string' && from.startsWith('/') && !from.startsWith('//') ? from : fallback;
}
