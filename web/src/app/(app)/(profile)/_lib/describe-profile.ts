import type { EarnedTrophy, Game, TrophySet } from '@openpsapp/contract/api';
import { monthKey, type TopSet } from '@openpsapp/core';
import type { MonthRange, PlatinumStatus, ProfileGame, TopGame } from '../_types/profile';

export const RECENT_GAMES = 6;
export const LATEST_TROPHIES = 4;

export function profileGames(games: Game[], sets: TrophySet[]): ProfileGame[] {
  const byId = new Map(sets.map((set) => [set.id, set]));
  return games
    .filter((game) => !game.hidden)
    .map((game) => {
      const set = game.trophySetId ? byId.get(game.trophySetId) : undefined;
      return {
        titleId: game.titleId,
        name: game.name,
        playtimeSeconds: game.playtimeSeconds,
        lastPlayedAt: game.lastPlayedAt,
        iconUrl: game.iconUrl,
        heroUrl: game.heroUrl,
        set: set ? { progress: set.progress, earned: set.earned, total: set.total } : null,
      };
    });
}

export function heroImage(games: ProfileGame[]): string | null {
  return games.find((game) => game.heroUrl)?.heroUrl ?? null;
}

export function platinumStatus(sets: TrophySet[], earned: EarnedTrophy[]): PlatinumStatus {
  const latest = earned.find((item) => item.trophy.grade === 'platinum');
  const candidates = sets.filter((set) => set.counts.platinum.total > 0 && set.counts.platinum.earned === 0);
  const closest = candidates.reduce<TrophySet | null>((best, set) => (!best || set.progress > best.progress ? set : best), null);
  return {
    count: sets.reduce((sum, set) => sum + set.counts.platinum.earned, 0),
    latest: latest?.setName ?? null,
    closest: closest ? { name: closest.name, progress: closest.progress } : null,
  };
}

export function monthRange(earned: EarnedTrophy[], now: Date, timeZone?: string): MonthRange {
  const current = monthKey(now, timeZone);
  const oldest = earned.reduce<string | null>((min, item) => {
    const at = item.trophy.earnedAt;
    return at && (!min || at < min) ? at : min;
  }, null);
  return { first: oldest ? monthKey(oldest, timeZone) : current, current };
}

export function topGame(top: TopSet | null, games: Game[], sets: TrophySet[]): TopGame | null {
  if (!top) return null;
  const game = top.titleId ? games.find((g) => g.titleId === top.titleId) : undefined;
  const set = sets.find((s) => s.id === top.setId);
  return {
    name: game?.name ?? top.setName,
    count: top.count,
    iconUrl: game?.iconUrl ?? set?.iconUrl ?? null,
    titleId: top.titleId,
  };
}
