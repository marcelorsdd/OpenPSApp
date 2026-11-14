import type { Profile } from '@openpsapp/contract/api';

export type TrophyTotals = Profile['totals']['trophies'];

export interface SetProgress {
  progress: number;
  earned: number;
  total: number;
}

export interface ProfileGame {
  titleId: string;
  name: string;
  playtimeSeconds: number;
  lastPlayedAt: string | null;
  iconUrl: string | null;
  heroUrl: string | null;
  set: SetProgress | null;
}

export interface PlatinumStatus {
  count: number;
  latest: string | null;
  closest: { name: string; progress: number } | null;
}

export interface MonthRange {
  first: string;
  current: string;
}

export interface TopGame {
  name: string;
  count: number;
  iconUrl: string | null;
  titleId: string | null;
}
