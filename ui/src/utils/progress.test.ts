import { describe, expect, it } from 'vitest';
import type { EarnedTrophy, GradeCount, Trophy, TrophySet } from '@/types/api';
import { groupByDay, isComplete, monthSummary } from '@/utils/progress';

const earnedItem = (setId: string, id: number, grade: Trophy['grade'], earnedAt: string, titleId: string | null = 'PPSA00001'): EarnedTrophy => ({
  setId,
  setName: `Set ${setId}`,
  titleId,
  trophy: { id, name: `T${id}`, description: '', grade, hidden: false, earned: true, earnedAt, iconUrl: null },
});

describe('monthSummary', () => {
  const earned = [
    earnedItem('A', 1, 'silver', '2026-11-08T05:00:00Z'),
    earnedItem('B', 7, 'bronze', '2026-11-07T05:00:00Z'),
    earnedItem('A', 2, 'bronze', '2026-11-03T05:00:00Z'),
    earnedItem('B', 8, 'gold', '2026-11-02T05:00:00Z'),
    earnedItem('A', 3, 'bronze', '2026-10-20T05:00:00Z'),
  ];
  const now = new Date('2026-11-14T12:00:00Z');

  it('counts the month by grade and breaks a tie for game of the month with the most recent trophy', () => {
    const s = monthSummary(earned, '2026-11', now, 'UTC');
    expect(s.total).toBe(4);
    expect(s.byGrade).toEqual({ platinum: 0, gold: 1, silver: 1, bronze: 2 });
    expect(s.topSet).toEqual({ setId: 'A', setName: 'Set A', titleId: 'PPSA00001', count: 2 });
  });

  it('prefers the set with more trophies in the month', () => {
    const more = [earnedItem('B', 9, 'bronze', '2026-11-01T05:00:00Z'), ...earned];
    expect(monthSummary(more, '2026-11', now, 'UTC').topSet?.setId).toBe('B');
  });

  it('paces the current month by days elapsed and past months by their length', () => {
    expect(monthSummary(earned, '2026-11', now, 'UTC').perWeek).toBe(2);
    expect(monthSummary(earned, '2026-10', now, 'UTC').perWeek).toBe(0.2);
  });

  it('returns an empty summary for a month without trophies', () => {
    const s = monthSummary(earned, '2026-09', now, 'UTC');
    expect(s.total).toBe(0);
    expect(s.topSet).toBeNull();
    expect(s.perWeek).toBe(0);
  });
});

const set = (id: string, earned: number, total: number, progress: number, platinum: GradeCount = { earned: 0, total: 1 }): TrophySet => ({
  id,
  name: `Set ${id}`,
  titleId: null,
  iconUrl: null,
  progress,
  earned,
  total,
  lastEarnedAt: null,
  counts: { platinum, gold: { earned: 0, total: 0 }, silver: { earned: 0, total: 0 }, bronze: { earned, total } },
});

describe('isComplete', () => {
  it('needs the platinum when the set has one, otherwise every trophy', () => {
    expect(isComplete(set('A', 9, 10, 90, { earned: 1, total: 1 }))).toBe(true);
    expect(isComplete(set('A', 10, 10, 100, { earned: 0, total: 1 }))).toBe(false);
    expect(isComplete(set('B', 5, 5, 100, { earned: 0, total: 0 }))).toBe(true);
    expect(isComplete(set('B', 4, 5, 80, { earned: 0, total: 0 }))).toBe(false);
  });
});

describe('groupByDay', () => {
  it('groups consecutive trophies by local day', () => {
    const groups = groupByDay(
      [
        earnedItem('A', 3, 'bronze', '2026-11-08T05:00:00Z'),
        earnedItem('A', 2, 'bronze', '2026-11-08T01:00:00Z'),
        earnedItem('A', 1, 'bronze', '2026-11-07T20:00:00Z'),
      ],
      'America/Sao_Paulo',
    );
    expect(groups.map((g) => [g.day, g.items.map((i) => i.trophy.id)])).toEqual([
      ['2026-11-08', [3]],
      ['2026-11-07', [2, 1]],
    ]);
  });
});
