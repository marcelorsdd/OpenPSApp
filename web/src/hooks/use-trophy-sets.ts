import type { TrophySet } from '@openpsapp/contract/api';
import { useQuery } from '@tanstack/react-query';
import { api } from '@/lib/api';
import { queryKeys } from '@/lib/query-keys';

export function useTrophySets() {
  return useQuery({ queryKey: queryKeys.trophySets, queryFn: () => api<TrophySet[]>('/trophy-sets') });
}
