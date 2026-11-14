import type { EarnedTrophy, Game, GradeCount, TrophySet } from '@openpsapp/contract/api';
import { describe, expect, it } from 'vitest';
import { gradeTotals, parseSetSort, setCards, timeline, visibleSets } from './describe-trophies';

const count = (earned: number, total: number): GradeCount => ({ earned, total });

const set = (id: string, name: string, progress: number, lastEarnedAt: string | null, platinum = count(0, 1)): TrophySet => ({
  id,
  name,
  titleId: `T${id}`,
  iconUrl: `/set/${id}`,
  progress,
  earned: 1,
  total: 10,
  lastEarnedAt,
  counts: { platinum, gold: count(0, 2), silver: count(1, 3), bronze: count(2, 4) },
});

const earned = (id: number, earnedAt: string): EarnedTrophy => ({
  setId: 'A',
  setName: 'Set A',
  titleId: 'TA',
  trophy: { id, name: `T${id}`, description: '', grade: 'bronze', hidden: false, earned: true, earnedAt, iconUrl: null },
});

const sets = [
  set('A', 'Wolverine', 34, '2026-11-08T10:00:00Z'),
  set('B', 'Astro', 100, '2026-10-01T10:00:00Z', count(1, 1)),
  set('C', 'Pragmata', 0, null),
];

describe('visibleSets', () => {
  it('sorts by the latest trophy, progress or name', () => {
    expect(visibleSets(sets, 'recent', false).map((s) => s.id)).toEqual(['A', 'B', 'C']);
    expect(visibleSets(sets, 'progress', false).map((s) => s.id)).toEqual(['B', 'A', 'C']);
    expect(visibleSets(sets, 'name', false).map((s) => s.id)).toEqual(['B', 'C', 'A']);
  });

  it('hides complete games on request', () => {
    expect(visibleSets(sets, 'recent', true).map((s) => s.id)).toEqual(['A', 'C']);
  });
});

describe('parseSetSort', () => {
  it('falls back to the latest trophy', () => {
    expect(parseSetSort('name')).toBe('name');
    expect(parseSetSort('nope')).toBe('recent');
  });
});

describe('gradeTotals', () => {
  it('adds earned and total per grade', () => {
    expect(gradeTotals(sets)).toEqual([
      { grade: 'platinum', earned: 1, total: 3 },
      { grade: 'gold', earned: 0, total: 6 },
      { grade: 'silver', earned: 3, total: 9 },
      { grade: 'bronze', earned: 6, total: 12 },
    ]);
  });
});

describe('setCards', () => {
  it('uses the game name and art when the set belongs to a game', () => {
    const games: Game[] = [
      {
        titleId: 'TA',
        name: "Marvel's Wolverine",
        playtimeSeconds: 1,
        lastPlayedAt: null,
        iconUrl: '/icon',
        heroUrl: '/hero',
        trophySetId: 'A',
        hidden: false,
      },
    ];
    const [a, b] = setCards(sets, games);
    expect(a).toMatchObject({ name: "Marvel's Wolverine", iconUrl: '/icon', heroUrl: '/hero' });
    expect(b).toMatchObject({ name: 'Astro', iconUrl: '/set/B', heroUrl: null });
  });
});

describe('timeline', () => {
  const items = [earned(1, '2026-11-08T12:00:00Z'), earned(2, '2026-11-08T11:00:00Z'), earned(3, '2026-11-07T12:00:00Z')];

  it('groups by day and keeps the day totals when cut short', () => {
    const days = timeline(items, 1, 'UTC');
    expect(days).toHaveLength(1);
    expect(days[0]).toMatchObject({ day: '2026-11-08', count: 2 });
    expect(days[0].items.map((i) => i.trophy.id)).toEqual([1]);
  });

  it('shows everything without a limit', () => {
    expect(timeline(items, null, 'UTC').map((d) => d.day)).toEqual(['2026-11-08', '2026-11-07']);
  });
});
