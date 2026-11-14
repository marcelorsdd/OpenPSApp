import type { EarnedTrophy, Game, TrophySet } from '@openpsapp/contract/api';
import { describe, expect, it } from 'vitest';
import { matchScore, searchGroups } from './search';

const game = (titleId: string, name: string, trophySetId: string | null, hidden = false): Game => ({
  titleId,
  name,
  playtimeSeconds: 60,
  lastPlayedAt: null,
  iconUrl: null,
  heroUrl: null,
  trophySetId,
  hidden,
});

const set = (id: string, progress: number): TrophySet => ({
  id,
  name: id,
  titleId: null,
  iconUrl: null,
  progress,
  earned: 0,
  total: 0,
  lastEarnedAt: null,
  counts: {
    platinum: { earned: 0, total: 0 },
    gold: { earned: 0, total: 0 },
    silver: { earned: 0, total: 0 },
    bronze: { earned: 0, total: 0 },
  },
});

const earned = (setId: string, id: number, name: string): EarnedTrophy => ({
  setId,
  setName: `Set ${setId}`,
  titleId: 'PPSA1',
  trophy: { id, name, description: '', grade: 'bronze', hidden: false, earned: true, earnedAt: null, iconUrl: null },
});

describe('matchScore', () => {
  it('matches every term without case or accents', () => {
    expect(matchScore('aparada', ['Aparada Aperfeiçoada', 'Wolverine'])).toBe(1);
    expect(matchScore('APERFEICOADA wolv', ['Aparada Aperfeiçoada', 'Wolverine'])).toBe(1);
    expect(matchScore('pragmata', ['Aparada Aperfeiçoada', 'Wolverine'])).toBe(0);
    expect(matchScore('  ', ['anything'])).toBe(1);
  });
});

describe('searchGroups', () => {
  const data = {
    games: [
      game('A', 'Alpha', 'S1'),
      game('B', 'Beta', null),
      game('C', 'Gamma', null),
      game('D', 'Delta', null),
      game('H', 'Hidden', null, true),
    ],
    sets: [set('S1', 34)],
    earned: [earned('S1', 1, 'One'), earned('S1', 2, 'Two'), earned('S1', 3, 'Three')],
  };

  it('shows a few recent items while the query is empty', () => {
    const groups = searchGroups(data, '');
    expect(groups.games.map((g) => g.titleId)).toEqual(['A', 'B', 'C']);
    expect(groups.trophies.map((t) => t.name)).toEqual(['One', 'Two']);
    expect(groups.totals).toEqual({ games: 4, trophies: 3 });
  });

  it('hands every visible item to the filter once there is a query', () => {
    const groups = searchGroups(data, 'ta');
    expect(groups.games).toHaveLength(4);
    expect(groups.trophies).toHaveLength(3);
  });

  it('adds the trophy progress of each game', () => {
    const [alpha, beta] = searchGroups(data, '').games;
    expect(alpha.progress).toBe(34);
    expect(beta.progress).toBeNull();
  });
});
