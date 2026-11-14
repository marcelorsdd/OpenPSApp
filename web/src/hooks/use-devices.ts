import type { DeviceInfo } from '@openpsapp/contract/pairing';
import { useQuery, type UseQueryOptions } from '@tanstack/react-query';
import { api } from '@/lib/api';
import { queryKeys } from '@/lib/query-keys';

export function useDevices(options: Pick<UseQueryOptions<DeviceInfo[]>, 'refetchInterval'> = {}) {
  return useQuery({
    queryKey: queryKeys.devices,
    queryFn: () => api<DeviceInfo[]>('/devices'),
    refetchInterval: options.refetchInterval,
  });
}
