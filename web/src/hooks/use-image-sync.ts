import type { ImageSync } from '@openpsapp/contract/api';
import { useQuery, useQueryClient } from '@tanstack/react-query';
import { useEffect, useRef } from 'react';
import { api } from '@/lib/api';
import { queryKeys } from '@/lib/query-keys';

export const IMAGE_SYNC_POLL_MS = 3000;

export function useImageSync() {
  const queryClient = useQueryClient();
  const query = useQuery({
    queryKey: queryKeys.images,
    queryFn: () => api<ImageSync>('/images'),
    refetchInterval: (q) => (q.state.data && q.state.data.stored < q.state.data.total ? IMAGE_SYNC_POLL_MS : false),
  });
  const stored = query.data?.stored;
  const seen = useRef(stored);
  useEffect(() => {
    if (stored === undefined || stored === seen.current) return;
    const first = seen.current === undefined;
    seen.current = stored;
    if (!first) void queryClient.invalidateQueries({ queryKey: queryKeys.library });
  }, [stored, queryClient]);
  return query.data ?? null;
}
