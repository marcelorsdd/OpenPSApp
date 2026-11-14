import type { Game, Trophy, TrophySet } from '@openpsapp/contract/api';
import { useQuery } from '@tanstack/react-query';
import { api } from '@/lib/api';
import { queryKeys } from '@/lib/query-keys';

const id = encodeURIComponent;

export function useGame(titleId: string) {
  const game = useQuery({ queryKey: queryKeys.game(titleId), queryFn: () => api<Game>(`/games/${id(titleId)}`) });
  const setId = game.data?.trophySetId ?? null;
  const set = useQuery({
    queryKey: queryKeys.trophySet(setId ?? ''),
    queryFn: () => api<TrophySet>(`/trophy-sets/${id(setId ?? '')}`),
    enabled: setId !== null,
  });
  const trophies = useQuery({
    queryKey: queryKeys.trophies(setId ?? ''),
    queryFn: () => api<Trophy[]>(`/trophy-sets/${id(setId ?? '')}/trophies`),
    enabled: setId !== null,
  });
  return { game, set, trophies, setId };
}
