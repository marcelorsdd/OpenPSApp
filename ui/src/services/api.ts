import type { EarnedTrophy, Game, Health, Profile, Trophy, TrophySet } from '@/types/api';

export type Endpoint<T> = string & { readonly __response?: T };

const endpoint = <T>(path: string) => path as Endpoint<T>;
const id = encodeURIComponent;

export const endpoints = {
  health: endpoint<Health>('/api/v1/health'),
  profile: endpoint<Profile>('/api/v1/profile'),
  games: endpoint<Game[]>('/api/v1/games'),
  game: (titleId: string) => endpoint<Game>(`/api/v1/games/${id(titleId)}`),
  trophySets: endpoint<TrophySet[]>('/api/v1/trophy-sets'),
  trophySet: (setId: string) => endpoint<TrophySet>(`/api/v1/trophy-sets/${id(setId)}`),
  trophies: (setId: string) => endpoint<Trophy[]>(`/api/v1/trophy-sets/${id(setId)}/trophies`),
  earnedTrophies: (limit?: number) =>
    endpoint<EarnedTrophy[]>(limit ? `/api/v1/trophies/earned?limit=${limit}` : '/api/v1/trophies/earned'),
};

async function request<T>(path: string, init?: RequestInit): Promise<T> {
  const res = await fetch(path, init);
  if (!res.ok) throw new Error(`HTTP ${res.status}`);
  return (await res.json()) as T;
}

export const get = <T>(endpoint: Endpoint<T>) => request<T>(endpoint);

export const setGameHidden = (titleId: string, hidden: boolean) =>
  request<Game>(`/api/v1/games/${id(titleId)}/${hidden ? 'hide' : 'unhide'}`, { method: 'POST' });
