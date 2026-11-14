import type { EarnedTrophy, Game, Health, Profile, Trophy, TrophySet } from '@/types/api';
import type { CloudStatus } from '@/types/cloud';

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
  cloud: endpoint<CloudStatus>('/api/v1/cloud'),
};

export class ApiError extends Error {
  status: number;
  code: string | null;

  constructor(status: number, code: string | null) {
    super(`HTTP ${status}`);
    this.status = status;
    this.code = code;
  }
}

async function request<T>(path: string, init: RequestInit = {}): Promise<T> {
  const res = await fetch(path, init);
  if (!res.ok) {
    const body = (await res.json().catch(() => null)) as { error?: string } | null;
    throw new ApiError(res.status, body?.error ?? null);
  }
  return (res.status === 204 ? undefined : await res.json()) as T;
}

const CONSOLE_HEADERS = { 'X-OpenPSApp': '1' };

const post = <T>(path: string) => request<T>(path, { method: 'POST', headers: CONSOLE_HEADERS });

export const get = <T>(endpoint: Endpoint<T>) => request<T>(endpoint);

export const setGameHidden = (titleId: string, hidden: boolean) => post<Game>(`/api/v1/games/${id(titleId)}/${hidden ? 'hide' : 'unhide'}`);

export const cloudAction = (action: 'link' | 'unlink' | 'sync') => post<CloudStatus>(`/api/v1/cloud/${action}`);
