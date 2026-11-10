import type { EarnedTrophy, Grade, TrophySet } from '@/types/api';
import type { DayGroup, MonthSummary, TopSet } from '@/types/progress';
import { dayKey, daysInMonth, monthKey } from '@/utils/dates';

export function monthSummary(earned: EarnedTrophy[], month: string, now: Date, timeZone?: string): MonthSummary {
  const byGrade: Record<Grade, number> = { platinum: 0, gold: 0, silver: 0, bronze: 0 };
  const bySet = new Map<string, TopSet & { latest: string }>();
  let total = 0;
  for (const e of earned) {
    const at = e.trophy.earnedAt;
    if (!at || monthKey(at, timeZone) !== month) continue;
    total++;
    if (e.trophy.grade !== 'unknown') byGrade[e.trophy.grade]++;
    const s = bySet.get(e.setId) ?? { setId: e.setId, setName: e.setName, titleId: e.titleId, count: 0, latest: '' };
    s.count++;
    if (at > s.latest) s.latest = at;
    bySet.set(e.setId, s);
  }
  let top: (TopSet & { latest: string }) | null = null;
  for (const s of bySet.values()) {
    if (!top || s.count > top.count || (s.count === top.count && s.latest > top.latest)) top = s;
  }
  const days = month === monthKey(now, timeZone) ? Number(dayKey(now, timeZone).slice(8)) : daysInMonth(month);
  return {
    total,
    byGrade,
    topSet: top && { setId: top.setId, setName: top.setName, titleId: top.titleId, count: top.count },
    perWeek: Math.round((total * 70) / days) / 10,
  };
}

export function isComplete(set: TrophySet): boolean {
  return set.counts.platinum.total > 0 ? set.counts.platinum.earned > 0 : set.total > 0 && set.earned === set.total;
}

export function groupByDay(earned: EarnedTrophy[], timeZone?: string): DayGroup[] {
  const groups: DayGroup[] = [];
  for (const item of earned) {
    if (!item.trophy.earnedAt) continue;
    const day = dayKey(item.trophy.earnedAt, timeZone);
    const last = groups[groups.length - 1];
    if (last && last.day === day) last.items.push(item);
    else groups.push({ day, items: [item] });
  }
  return groups;
}
