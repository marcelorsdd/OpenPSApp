import type { Trophy } from '@openpsapp/contract/api';
import type { TrophyFilter, TrophySort } from '@openpsapp/core';

export interface TrophyRowView {
  id: number;
  grade: Trophy['grade'];
  name: string;
  description: string;
  iconUrl: string | null;
  earned: boolean;
  earnedAt: string | null;
  masked: boolean;
}

export interface TrophyCounts {
  all: number;
  earned: number;
  locked: number;
  hidden: number;
}

export interface TrophyListOptions {
  filter: TrophyFilter;
  sort: TrophySort;
  showHidden: boolean;
  revealed: ReadonlySet<number>;
}
