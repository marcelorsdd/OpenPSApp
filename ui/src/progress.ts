import type { EarnedTrophy, Game, Grade, TrophySet } from './api';

const formatters = new Map<string, Intl.DateTimeFormat>();

function formatter(timeZone?: string): Intl.DateTimeFormat {
  const key = timeZone ?? '';
  let f = formatters.get(key);
  if (!f) {
    f = new Intl.DateTimeFormat('en-US', { timeZone, year: 'numeric', month: '2-digit', day: '2-digit' });
    formatters.set(key, f);
  }
  return f;
}

export function dayKey(date: string | Date, timeZone?: string): string {
  const parts = formatter(timeZone).formatToParts(typeof date === 'string' ? new Date(date) : date);
  const part = (type: string) => parts.find((p) => p.type === type)?.value ?? '';
  return `${part('year')}-${part('month')}-${part('day')}`;
}

export function monthKey(date: string | Date, timeZone?: string): string {
  return dayKey(date, timeZone).slice(0, 7);
}

function splitMonth(month: string): [number, number] {
  const [y, m] = month.split('-').map(Number);
  return [y, m];
}

export function shiftMonth(month: string, delta: number): string {
  const [y, m] = splitMonth(month);
  const d = new Date(Date.UTC(y, m - 1 + delta, 1));
  return `${d.getUTCFullYear()}-${String(d.getUTCMonth() + 1).padStart(2, '0')}`;
}

export function daysInMonth(month: string): number {
  const [y, m] = splitMonth(month);
  return new Date(Date.UTC(y, m, 0)).getUTCDate();
}

export interface TopSet {
  setId: string;
  setName: string;
  titleId: string | null;
  count: number;
}

export interface MonthSummary {
  total: number;
  byGrade: Record<Grade, number>;
  topSet: TopSet | null;
  perWeek: number;
}

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

export type Milestone =
  | { kind: 'firstTrophy'; item: EarnedTrophy }
  | { kind: 'firstGrade'; grade: 'silver' | 'gold' | 'platinum'; item: EarnedTrophy }
  | { kind: 'trophyCount'; count: number; item: EarnedTrophy }
  | { kind: 'platinumCount'; count: number; item: EarnedTrophy }
  | { kind: 'complete'; item: EarnedTrophy };

const TROPHY_COUNTS = [10, 25, 50, 100, 250, 500, 1000];
const PLATINUM_COUNTS = [5, 10, 25, 50, 100];

function isTrophyCount(n: number): boolean {
  return TROPHY_COUNTS.includes(n) || (n > 1000 && n % 1000 === 0);
}

function chronological(earned: EarnedTrophy[]): EarnedTrophy[] {
  return earned
    .filter((e) => e.trophy.earnedAt)
    .sort(
      (a, b) =>
        (a.trophy.earnedAt ?? '').localeCompare(b.trophy.earnedAt ?? '') ||
        a.setId.localeCompare(b.setId) ||
        a.trophy.id - b.trophy.id,
    );
}

export function milestones(earned: EarnedTrophy[], sets: TrophySet[]): Milestone[] {
  const totals = new Map(sets.map((s) => [s.id, s.total]));
  const perSet = new Map<string, number>();
  const seen = new Set<Grade>();
  const out: Milestone[] = [];
  let count = 0;
  let platinums = 0;
  for (const item of chronological(earned)) {
    const grade = item.trophy.grade;
    count++;
    if (count === 1) out.push({ kind: 'firstTrophy', item });
    if ((grade === 'silver' || grade === 'gold' || grade === 'platinum') && !seen.has(grade)) {
      seen.add(grade);
      out.push({ kind: 'firstGrade', grade, item });
    }
    if (isTrophyCount(count)) out.push({ kind: 'trophyCount', count, item });
    if (grade === 'platinum' && PLATINUM_COUNTS.includes(++platinums)) out.push({ kind: 'platinumCount', count: platinums, item });
    const inSet = (perSet.get(item.setId) ?? 0) + 1;
    perSet.set(item.setId, inSet);
    if (inSet === totals.get(item.setId)) out.push({ kind: 'complete', item });
  }
  return out.reverse();
}

export function nextMilestone(earnedCount: number): { count: number; remaining: number } {
  const count = TROPHY_COUNTS.find((c) => c > earnedCount) ?? (Math.floor(earnedCount / 1000) + 1) * 1000;
  return { count, remaining: count - earnedCount };
}

export interface Stats {
  minutesPerTrophy: number | null;
  averageCompletion: number | null;
  platinumSets: number;
  startedSets: number;
}

export function stats(games: Game[], sets: TrophySet[]): Stats {
  const seconds = games.filter((g) => !g.hidden).reduce((sum, g) => sum + g.playtimeSeconds, 0);
  const started = sets.filter((s) => s.earned > 0);
  const earned = started.reduce((sum, s) => sum + s.earned, 0);
  return {
    minutesPerTrophy: earned > 0 ? Math.round(seconds / 60 / earned) : null,
    averageCompletion: started.length > 0 ? Math.round(started.reduce((sum, s) => sum + s.progress, 0) / started.length) : null,
    platinumSets: started.filter((s) => s.counts.platinum.earned > 0).length,
    startedSets: started.length,
  };
}

export function isComplete(set: TrophySet): boolean {
  return set.counts.platinum.total > 0 ? set.counts.platinum.earned > 0 : set.total > 0 && set.earned === set.total;
}

export interface DayGroup {
  day: string;
  items: EarnedTrophy[];
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
