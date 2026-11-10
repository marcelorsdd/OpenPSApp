import { describe, expect, it } from 'vitest';
import type { Trophy } from '@/types/api';
import { filterTrophies, recentlyEarned, sortTrophies, trophyView } from '@/utils/trophies';

const trophy = (id: number, grade: Trophy['grade'], earnedAt: string | null): Trophy => ({
  id,
  name: `T${id}`,
  description: '',
  grade,
  hidden: false,
  earned: earnedAt !== null,
  earnedAt,
  iconUrl: null,
});

const list = [
  trophy(0, 'platinum', null),
  trophy(1, 'bronze', '2026-10-20T10:00:00Z'),
  trophy(2, 'gold', null),
  trophy(3, 'bronze', '2026-11-08T10:00:00Z'),
  trophy(4, 'silver', '2026-11-01T10:00:00Z'),
];

describe('sortTrophies', () => {
  it('keeps the game order by default', () => {
    expect(sortTrophies([...list].reverse(), 'default').map((t) => t.id)).toEqual([0, 1, 2, 3, 4]);
  });

  it('puts the most recently earned first, then the locked ones in game order', () => {
    expect(sortTrophies(list, 'earned').map((t) => t.id)).toEqual([3, 4, 1, 0, 2]);
  });

  it('groups by trophy type from platinum to bronze', () => {
    expect(sortTrophies(list, 'grade').map((t) => t.id)).toEqual([0, 2, 4, 1, 3]);
  });

  it('does not reorder its input', () => {
    const input = [...list];
    sortTrophies(input, 'earned');
    expect(input.map((t) => t.id)).toEqual([0, 1, 2, 3, 4]);
  });
});

describe('filterTrophies', () => {
  it('keeps all, only earned or only locked', () => {
    expect(filterTrophies(list, 'all')).toHaveLength(5);
    expect(filterTrophies(list, 'earned').map((t) => t.id)).toEqual([1, 3, 4]);
    expect(filterTrophies(list, 'locked').map((t) => t.id)).toEqual([0, 2]);
  });
});

describe('recentlyEarned', () => {
  it('lists the newest earned trophies up to the limit', () => {
    expect(recentlyEarned(list, 2).map((t) => t.id)).toEqual([3, 4]);
    expect(recentlyEarned(list, 6).map((t) => t.id)).toEqual([3, 4, 1]);
  });
});

describe('trophyView', () => {
  const hidden: Trophy = {
    id: 7,
    name: 'Secret',
    description: 'Spoiler',
    grade: 'gold',
    hidden: true,
    earned: false,
    earnedAt: null,
    iconUrl: '/api/v1/icons/abc',
  };

  it('masks hidden trophies that are not earned', () => {
    const v = trophyView(hidden, false);
    expect(v.masked).toBe(true);
    expect(v.name).toBe('');
    expect(v.description).toBe('');
    expect(v.iconUrl).toBeNull();
  });

  it('shows hidden trophies once revealed or earned', () => {
    expect(trophyView(hidden, true).name).toBe('Secret');
    expect(trophyView({ ...hidden, earned: true }, false).masked).toBe(false);
  });

  it('never masks regular trophies', () => {
    expect(trophyView({ ...hidden, hidden: false }, false).name).toBe('Secret');
  });
});
