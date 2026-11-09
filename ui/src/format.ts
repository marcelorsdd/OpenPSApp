import type { Trophy } from './api';

export function formatDuration(seconds: number): string {
  if (seconds <= 0) return '0 min';
  if (seconds < 60) return '< 1 min';
  const minutes = Math.floor(seconds / 60);
  if (minutes < 60) return `${minutes} min`;
  const hours = Math.floor(minutes / 60);
  return `${hours} h ${String(minutes % 60).padStart(2, '0')} min`;
}

export function formatDate(iso: string | null, locale: string, timeZone?: string): string {
  if (!iso) return '—';
  return new Date(iso).toLocaleDateString(locale, { timeZone, day: '2-digit', month: '2-digit', year: 'numeric' });
}

export interface TrophyView {
  masked: boolean;
  name: string;
  description: string;
  iconUrl: string | null;
}

export function trophyView(t: Trophy, revealed: boolean): TrophyView {
  if (t.hidden && !t.earned && !revealed) return { masked: true, name: '', description: '', iconUrl: null };
  return { masked: false, name: t.name, description: t.description, iconUrl: t.iconUrl };
}

export type TrophySort = 'default' | 'earned' | 'grade';
export type TrophyFilter = 'all' | 'earned' | 'locked';

const GRADE_ORDER: Record<Trophy['grade'], number> = { platinum: 0, gold: 1, silver: 2, bronze: 3, unknown: 4 };

export function sortTrophies(list: Trophy[], sort: TrophySort): Trophy[] {
  const byId = (a: Trophy, b: Trophy) => a.id - b.id;
  const sorted = [...list];
  if (sort === 'default') return sorted.sort(byId);
  if (sort === 'grade') return sorted.sort((a, b) => GRADE_ORDER[a.grade] - GRADE_ORDER[b.grade] || byId(a, b));
  return sorted.sort((a, b) => {
    if (a.earned !== b.earned) return a.earned ? -1 : 1;
    if (a.earned && a.earnedAt !== b.earnedAt) return (b.earnedAt ?? '').localeCompare(a.earnedAt ?? '');
    return byId(a, b);
  });
}

export function filterTrophies(list: Trophy[], filter: TrophyFilter): Trophy[] {
  if (filter === 'all') return list;
  return list.filter((t) => t.earned === (filter === 'earned'));
}
