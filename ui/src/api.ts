import { useCallback, useEffect, useState } from 'react';

export type Grade = 'platinum' | 'gold' | 'silver' | 'bronze';
export const GRADES: Grade[] = ['platinum', 'gold', 'silver', 'bronze'];

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

export interface RecentTrophy {
  setId: string;
  setName: string;
  titleId: string | null;
  trophy: Trophy;
}

const id = encodeURIComponent;

export const paths = {
  health: '/api/v1/health',
  profile: '/api/v1/profile',
  games: '/api/v1/games',
  game: (titleId: string) => `/api/v1/games/${id(titleId)}`,
  hide: (titleId: string, hide: boolean) => `/api/v1/games/${id(titleId)}/${hide ? 'hide' : 'unhide'}`,
  trophySets: '/api/v1/trophy-sets',
  trophySet: (setId: string) => `/api/v1/trophy-sets/${id(setId)}`,
  trophies: (setId: string) => `/api/v1/trophy-sets/${id(setId)}/trophies`,
  recentTrophies: (limit: number) => `/api/v1/trophies/recent?limit=${limit}`,
};

async function fetchJson<T>(path: string, init?: RequestInit): Promise<T> {
  const res = await fetch(path, init);
  if (!res.ok) throw new Error(`HTTP ${res.status}`);
  return (await res.json()) as T;
}

export const post = <T>(path: string) => fetchJson<T>(path, { method: 'POST' });

export interface ApiState<T> {
  data: T | null;
  error: string | null;
  reload: () => void;
}

export function useApi<T>(path: string | null): ApiState<T> {
  const [data, setData] = useState<T | null>(null);
  const [error, setError] = useState<string | null>(null);
  const [attempt, setAttempt] = useState(0);
  const reload = useCallback(() => setAttempt((n) => n + 1), []);

  useEffect(() => {
    if (path === null) return;
    let cancelled = false;
    setError(null);
    fetchJson<T>(path)
      .then((body) => !cancelled && setData(body))
      .catch((e: Error) => !cancelled && setError(e.message));
    return () => {
      cancelled = true;
    };
  }, [path, attempt]);

  return { data, error, reload };
}

export function combine(...states: ApiState<unknown>[]) {
  return {
    error: states.find((s) => s.error)?.error ?? null,
    reload: () => states.forEach((s) => s.reload()),
  };
}
