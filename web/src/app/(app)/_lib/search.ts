import type { EarnedTrophy, Game, TrophySet } from '@openpsapp/contract/api';
import type { SearchGame, SearchGroups, SearchTrophy } from '../_types/search';

export const IDLE_GAMES = 3;
export const IDLE_TROPHIES = 2;

export function foldText(text: string): string {
  return text.normalize('NFD').replace(/[̀-ͯ]/g, '').toLowerCase();
}

export function matchScore(search: string, keywords: string[] = []): number {
  const terms = foldText(search).split(/\s+/).filter(Boolean);
  if (terms.length === 0) return 1;
  const haystack = foldText(keywords.join(' '));
  return terms.every((term) => haystack.includes(term)) ? 1 : 0;
}

export function searchGroups(data: { games: Game[]; sets: TrophySet[]; earned: EarnedTrophy[] }, query: string): SearchGroups {
  const progress = new Map(data.sets.map((set) => [set.id, set.progress]));
  const visible = data.games.filter((game) => !game.hidden);
  const games: SearchGame[] = visible.map((game) => ({
    value: `game:${game.titleId}`,
    titleId: game.titleId,
    name: game.name,
    iconUrl: game.iconUrl,
    progress: game.trophySetId ? (progress.get(game.trophySetId) ?? null) : null,
    playtimeSeconds: game.playtimeSeconds,
  }));
  const trophies: SearchTrophy[] = data.earned.map((item) => ({
    value: `trophy:${item.setId}:${item.trophy.id}`,
    name: item.trophy.name,
    game: item.setName,
    grade: item.trophy.grade,
    iconUrl: item.trophy.iconUrl,
    titleId: item.titleId,
  }));
  const idle = query.trim().length === 0;
  return {
    games: idle ? games.slice(0, IDLE_GAMES) : games,
    trophies: idle ? trophies.slice(0, IDLE_TROPHIES) : trophies,
    totals: { games: games.length, trophies: trophies.length },
  };
}
