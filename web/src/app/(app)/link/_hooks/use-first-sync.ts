import { useQueryClient } from '@tanstack/react-query';
import { useEffect } from 'react';
import { useDevices } from '@/hooks/use-devices';
import { queryKeys } from '@/lib/query-keys';
import { FIRST_SYNC_POLL_MS, FIRST_SYNC_STEPS, firstSyncPhase } from '../_lib/describe-link';

export function useFirstSync(previousIds: string[] | null) {
  const queryClient = useQueryClient();
  const watching = previousIds !== null;
  const devices = useDevices({
    refetchInterval: (query) => (watching && firstSyncPhase(query.state.data, previousIds ?? []) !== 'synced' ? FIRST_SYNC_POLL_MS : false),
  });
  const phase = firstSyncPhase(devices.data, previousIds ?? []);
  useEffect(() => {
    if (watching && phase === 'synced') void queryClient.invalidateQueries({ queryKey: queryKeys.library });
  }, [watching, phase, queryClient]);
  return { phase, step: FIRST_SYNC_STEPS.indexOf(phase) + 1, steps: FIRST_SYNC_STEPS.length };
}
