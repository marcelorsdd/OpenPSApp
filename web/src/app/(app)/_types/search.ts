import type { Grade } from '@openpsapp/contract/api';

export interface SearchGame {
  value: string;
  titleId: string;
  name: string;
  iconUrl: string | null;
  progress: number | null;
  playtimeSeconds: number;
}

export interface SearchTrophy {
  value: string;
  name: string;
  game: string;
  grade: Grade | 'unknown';
  iconUrl: string | null;
  titleId: string | null;
}

export interface SearchGroups {
  games: SearchGame[];
  trophies: SearchTrophy[];
  totals: { games: number; trophies: number };
}
