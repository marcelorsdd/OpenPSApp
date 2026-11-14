import type { EarnedTrophy, Game, GradeCount, TrophySet } from '@openpsapp/contract/api';
import { describe, expect, it } from 'vitest';
import { heroImage, monthRange, platinumStatus, profileGames, topGame } from './describe-profile';

const game = (titleId: string, overrides: Partial<Game> = {}): Game => ({
  titleId,
  name: `Game ${titleId}`,
  playtimeSeconds: 600,
  lastPlayedAt: null,
  iconUrl: `/icon/${titleId}`,
  heroUrl: null,
  trophySetId: null,
  hidden: false,
  ...overrides,
});

const set = (id: string, progress: number, platinum: GradeCount): TrophySet => ({
  id,
  name: `Set ${id}`,
  titleId: null,
  iconUrl: `/set/${id}`,
  progress,
  earned: 1,
  total: 10,
  lastEarnedAt: null,
  counts: { platinum, gold: { earned: 0, total: 1 }, silver: { earned: 0, total: 1 }, bronze: { earned: 1, total: 7 } },
});

const earned = (setId: string, grade: EarnedTrophy['trophy']['grade'], earnedAt: string): EarnedTrophy => ({
  setId,
  setName: `Set ${setId}`,
  titleId: null,
  trophy: { id: 1, name: 'T', description: '', grade, hidden: false, earned: true, earnedAt, iconUrl: null },
});

describe('profileGames', () => {
  it('drops hidden games and joins the trophy progress', () => {
    const games = profileGames(
      [game('A', { trophySetId: 'S1' }), game('B', { hidden: true }), game('C')],
      [set('S1', 34, { earned: 0, total: 1 })],
    );
    expect(games.map((g) => g.titleId)).toEqual(['A', 'C']);
    expect(games[0].set).toEqual({ progress: 34, earned: 1, total: 10 });
    expect(games[1].set).toBeNull();
  });
});

describe('heroImage', () => {
  it('uses the key art of the most recent game that has one', () => {
    const games = profileGames([game('A'), game('B', { heroUrl: '/hero/B' }), game('C', { heroUrl: '/hero/C' })], []);
    expect(heroImage(games)).toBe('/hero/B');
    expect(heroImage([])).toBeNull();
  });
});

describe('platinumStatus', () => {
  it('counts platinums and finds the closest unearned one', () => {
    const sets = [set('A', 34, { earned: 0, total: 1 }), set('B', 60, { earned: 0, total: 1 }), set('C', 90, { earned: 0, total: 0 })];
    expect(platinumStatus(sets, [])).toEqual({ count: 0, latest: null, closest: { name: 'Set B', progress: 60 } });
  });

  it('names the latest platinum', () => {
    const sets = [set('A', 100, { earned: 1, total: 1 })];
    expect(platinumStatus(sets, [earned('A', 'platinum', '2026-11-08T10:00:00Z')])).toEqual({ count: 1, latest: 'Set A', closest: null });
  });
});

describe('monthRange', () => {
  it('goes from the month of the oldest trophy to the current month', () => {
    const now = new Date('2026-11-14T12:00:00Z');
    expect(monthRange([earned('A', 'bronze', '2026-11-08T10:00:00Z'), earned('A', 'bronze', '2026-09-02T10:00:00Z')], now, 'UTC')).toEqual({
      first: '2026-09',
      current: '2026-11',
    });
    expect(monthRange([], now, 'UTC')).toEqual({ first: '2026-11', current: '2026-11' });
  });
});

describe('topGame', () => {
  it('prefers the game name and icon over the trophy set', () => {
    const games = [game('PPSA1', { name: "Marvel's Wolverine" })];
    const sets = [set('S1', 34, { earned: 0, total: 1 })];
    expect(topGame({ setId: 'S1', setName: 'Wolverine', titleId: 'PPSA1', count: 15 }, games, sets)).toEqual({
      name: "Marvel's Wolverine",
      count: 15,
      iconUrl: '/icon/PPSA1',
      titleId: 'PPSA1',
    });
    expect(topGame({ setId: 'S1', setName: 'Wolverine', titleId: null, count: 2 }, games, sets)?.iconUrl).toBe('/set/S1');
    expect(topGame(null, games, sets)).toBeNull();
  });
});
