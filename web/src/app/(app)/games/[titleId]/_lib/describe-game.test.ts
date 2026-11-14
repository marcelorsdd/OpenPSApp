import type { Trophy } from '@openpsapp/contract/api';
import { describe, expect, it } from 'vitest';
import { backTarget, parseTrophySort, recentTrophies, trophyCounts, trophyRows } from './describe-game';

const trophy = (id: number, grade: Trophy['grade'], earnedAt: string | null, hidden = false): Trophy => ({
  id,
  name: `T${id}`,
  description: `D${id}`,
  grade,
  hidden,
  earned: earnedAt !== null,
  earnedAt,
  iconUrl: `/icon/${id}`,
});

const list = [
  trophy(0, 'platinum', null),
  trophy(1, 'bronze', '2026-11-07T10:00:00Z'),
  trophy(2, 'gold', null, true),
  trophy(3, 'silver', '2026-11-08T10:00:00Z', true),
];

describe('trophyCounts', () => {
  it('counts earned, missing and hidden trophies still locked', () => {
    expect(trophyCounts(list)).toEqual({ all: 4, earned: 2, locked: 2, hidden: 1 });
  });
});

describe('trophyRows', () => {
  const base = { filter: 'all' as const, sort: 'default' as const, showHidden: false, revealed: new Set<number>() };

  it('masks hidden locked trophies until revealed', () => {
    const rows = trophyRows(list, base);
    expect(rows.map((r) => r.masked)).toEqual([false, false, true, false]);
    expect(rows[2]).toMatchObject({ name: '', iconUrl: null });
    expect(trophyRows(list, { ...base, revealed: new Set([2]) })[2]).toMatchObject({ masked: false, name: 'T2' });
    expect(trophyRows(list, { ...base, showHidden: true }).every((r) => !r.masked)).toBe(true);
  });

  it('filters and sorts', () => {
    expect(trophyRows(list, { ...base, filter: 'earned', sort: 'earned' }).map((r) => r.id)).toEqual([3, 1]);
    expect(trophyRows(list, { ...base, filter: 'locked' }).map((r) => r.id)).toEqual([0, 2]);
    expect(trophyRows(list, { ...base, sort: 'grade' }).map((r) => r.id)).toEqual([0, 2, 3, 1]);
  });
});

describe('recentTrophies', () => {
  it('lists the newest earned trophies', () => {
    expect(recentTrophies(list).map((t) => t.id)).toEqual([3, 1]);
  });
});

describe('parseTrophySort', () => {
  it('keeps known sorts only', () => {
    expect(parseTrophySort('grade')).toBe('grade');
    expect(parseTrophySort(1)).toBe('default');
  });
});

describe('backTarget', () => {
  it('goes back to the page that linked here, never off the site', () => {
    expect(backTarget({ from: '/trophies' }, '/')).toBe('/trophies');
    expect(backTarget({ from: '//evil.example' }, '/')).toBe('/');
    expect(backTarget({ from: 'https://evil.example' }, '/')).toBe('/');
    expect(backTarget(null, '/')).toBe('/');
  });
});
