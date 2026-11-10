export type Grade = 'platinum' | 'gold' | 'silver' | 'bronze';

export interface Health {
  ok: boolean;
  version: string;
  source: string;
  capturedAt: string;
  language: string;
}

export interface Profile {
  user: { id: string; name: string } | null;
  totals: {
    games: number;
    playtimeSeconds: number;
    trophies: Record<Grade, number> & { earned: number; total: number };
  };
}

export interface Game {
  titleId: string;
  name: string;
  playtimeSeconds: number;
  lastPlayedAt: string | null;
  iconUrl: string | null;
  heroUrl: string | null;
  trophySetId: string | null;
  hidden: boolean;
}

export interface GradeCount {
  earned: number;
  total: number;
}

export interface TrophySet {
  id: string;
  name: string;
  titleId: string | null;
  iconUrl: string | null;
  progress: number;
  earned: number;
  total: number;
  lastEarnedAt: string | null;
  counts: Record<Grade, GradeCount>;
}

export interface Trophy {
  id: number;
  name: string;
  description: string;
  grade: Grade | 'unknown';
  hidden: boolean;
  earned: boolean;
  earnedAt: string | null;
  iconUrl: string | null;
}

export interface EarnedTrophy {
  setId: string;
  setName: string;
  titleId: string | null;
  trophy: Trophy;
}
