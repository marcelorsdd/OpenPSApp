export const queryKeys = {
  health: ['health'] as const,
  library: ['library'] as const,
  profile: ['library', 'profile'] as const,
  games: ['library', 'games'] as const,
  game: (titleId: string) => ['library', 'games', titleId] as const,
  trophySets: ['library', 'trophy-sets'] as const,
  trophySet: (setId: string) => ['library', 'trophy-sets', setId] as const,
  trophies: (setId: string) => ['library', 'trophy-sets', setId, 'trophies'] as const,
  earned: ['library', 'earned'] as const,
  devices: ['devices'] as const,
  images: ['images'] as const,
};
