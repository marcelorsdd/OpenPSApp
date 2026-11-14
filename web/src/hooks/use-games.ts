import type { Game } from '@openpsapp/contract/api';
import { useQuery } from '@tanstack/react-query';
import { api } from '@/lib/api';
import { queryKeys } from '@/lib/query-keys';

export function useGames() {
  return useQuery({ queryKey: queryKeys.games, queryFn: () => api<Game[]>('/games') });
}
