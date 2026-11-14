import type { Profile } from '@openpsapp/contract/api';
import { useQuery } from '@tanstack/react-query';
import { api } from '@/lib/api';
import { queryKeys } from '@/lib/query-keys';

export function useProfile() {
  return useQuery({ queryKey: queryKeys.profile, queryFn: () => api<Profile>('/profile') });
}
