import type { Grade, Trophy } from '@/types/api';
import type { TrophyFilter, TrophySort, TrophyView } from '@/types/trophies';

export const GRADES: Grade[] = ['platinum', 'gold', 'silver', 'bronze'];

export function trophyView(t: Trophy, revealed: boolean): TrophyView {
  if (t.hidden && !t.earned && !revealed) return { masked: true, name: '', description: '', iconUrl: null };
  return { masked: false, name: t.name, description: t.description, iconUrl: t.iconUrl };
}

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

export function recentlyEarned(list: Trophy[], limit: number): Trophy[] {
  return sortTrophies(filterTrophies(list, 'earned'), 'earned').slice(0, limit);
}
