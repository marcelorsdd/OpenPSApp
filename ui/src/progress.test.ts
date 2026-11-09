import { describe, expect, it } from 'vitest';
import type { EarnedTrophy, Game, GradeCount, Trophy, TrophySet } from './api';
import { dayKey, daysInMonth, groupByDay, isComplete, milestones, monthKey, monthSummary, nextMilestone, shiftMonth, stats } from './progress';

const earnedItem = (setId: string, id: number, grade: Trophy['grade'], earnedAt: string, titleId: string | null = 'PPSA00001'): EarnedTrophy => ({
  setId,
  setName: `Set ${setId}`,
  titleId,
  trophy: { id, name: `T${id}`, description: '', grade, hidden: false, earned: true, earnedAt, iconUrl: null },
});

describe('date keys', () => {
  it('cut days and months in the given time zone', () => {
    expect(dayKey('2026-11-01T02:00:00Z', 'UTC')).toBe('2026-11-01');
    expect(dayKey('2026-11-01T02:00:00Z', 'America/Sao_Paulo')).toBe('2026-10-31');
    expect(monthKey('2026-11-01T02:00:00Z', 'America/Sao_Paulo')).toBe('2026-10');
    expect(monthKey(new Date('2026-11-09T12:00:00Z'), 'UTC')).toBe('2026-11');
  });

  it('shift months across years', () => {
    expect(shiftMonth('2026-01', -1)).toBe('2025-12');
    expect(shiftMonth('2026-12', 1)).toBe('2027-01');
    expect(shiftMonth('2026-11', 0)).toBe('2026-11');
  });

  it('count days in a month', () => {
    expect(daysInMonth('2026-02')).toBe(28);
    expect(daysInMonth('2028-02')).toBe(29);
    expect(daysInMonth('2026-10')).toBe(31);
  });
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

const game = (titleId: string, playtimeSeconds: number, hidden = false): Game => ({
  titleId,
  name: titleId,
  playtimeSeconds,
  lastPlayedAt: null,
  iconUrl: null,
  heroUrl: null,
  trophySetId: null,
  hidden,
});

describe('milestones', () => {
  const day = (n: number) => `2026-10-${String(n).padStart(2, '0')}T10:00:00Z`;
  const earned = [
    ...Array.from({ length: 9 }, (_, i) => earnedItem('A', i + 1, 'bronze', day(i + 1))),
    earnedItem('A', 10, 'silver', day(10)),
    earnedItem('B', 1, 'gold', day(11)),
    earnedItem('B', 2, 'platinum', day(12)),
  ].reverse();

  it('lists them newest first with the trophy that reached each one', () => {
    const list = milestones(earned, [set('A', 10, 10, 100), set('B', 2, 3, 50)]);
    expect(list.map((m) => m.kind)).toEqual(['firstGrade', 'firstGrade', 'complete', 'trophyCount', 'firstGrade', 'firstTrophy']);
    expect(list[0]).toMatchObject({ kind: 'firstGrade', grade: 'platinum' });
    expect(list[2].item.setId).toBe('A');
    expect(list[3]).toMatchObject({ kind: 'trophyCount', count: 10 });
    expect(list[5].item.trophy.earnedAt).toBe(day(1));
  });

  it('counts platinums from the fifth', () => {
    const plats = Array.from({ length: 5 }, (_, i) => earnedItem(`P${i}`, 0, 'platinum', day(i + 1)));
    expect(milestones(plats, []).filter((m) => m.kind === 'platinumCount')).toHaveLength(1);
  });

  it('points to the next trophy count', () => {
    expect(nextMilestone(0)).toEqual({ count: 10, remaining: 10 });
    expect(nextMilestone(21)).toEqual({ count: 25, remaining: 4 });
    expect(nextMilestone(1000)).toEqual({ count: 2000, remaining: 1000 });
    expect(nextMilestone(2500)).toEqual({ count: 3000, remaining: 500 });
  });
});

describe('stats', () => {
  it('derive time per trophy, completion and platinums', () => {
    const s = stats(
      [game('A', 6000), game('B', 3000), game('H', 99999, true)],
      [set('A', 10, 10, 100, { earned: 1, total: 1 }), set('B', 5, 20, 30), set('C', 0, 5, 0)],
    );
    expect(s).toEqual({ minutesPerTrophy: 10, averageCompletion: 65, platinumSets: 1, startedSets: 2 });
  });

  it('stay empty without trophies', () => {
    expect(stats([game('A', 600)], [set('C', 0, 5, 0)])).toEqual({
      minutesPerTrophy: null,
      averageCompletion: null,
      platinumSets: 0,
      startedSets: 0,
    });
  });
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
