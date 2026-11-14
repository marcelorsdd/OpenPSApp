import type { EarnedTrophy } from '@openpsapp/contract/api';
import { useQuery } from '@tanstack/react-query';
import { api } from '@/lib/api';
import { queryKeys } from '@/lib/query-keys';

export function useEarnedTrophies() {
  return useQuery({ queryKey: queryKeys.earned, queryFn: () => api<EarnedTrophy[]>('/trophies/earned') });
}
