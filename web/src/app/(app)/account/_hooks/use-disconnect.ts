import { useMutation, useQueryClient } from '@tanstack/react-query';
import { useState } from 'react';
import { useTranslation } from 'react-i18next';
import { toast } from 'sonner';
import { apiDelete } from '@/lib/api';
import { queryKeys } from '@/lib/query-keys';
import { disconnectPath } from '../_lib/describe-account';

export function useDisconnect(deviceId: string | null) {
  const { t } = useTranslation();
  const queryClient = useQueryClient();
  const [open, setOpen] = useState(false);
  const [purge, setPurge] = useState(false);
  const mutation = useMutation({
    mutationFn: (variables: { deviceId: string; purge: boolean }) => apiDelete(disconnectPath(variables.deviceId, variables.purge)),
    onSuccess: async (_, variables) => {
      toast.success(variables.purge ? t('account.disconnect.doneWithData') : t('account.disconnect.done'));
      setOpen(false);
      setPurge(false);
      await queryClient.invalidateQueries({ queryKey: queryKeys.devices });
      if (variables.purge) await queryClient.resetQueries({ queryKey: queryKeys.library });
      else await queryClient.invalidateQueries({ queryKey: queryKeys.profile });
    },
    onError: () => toast.error(t('account.disconnect.failed')),
  });
  return {
    open,
    purge,
    pending: mutation.isPending,
    setPurge,
    setOpen: (next: boolean) => {
      if (mutation.isPending) return;
      setOpen(next);
      if (!next) setPurge(false);
    },
    confirm: () => {
      if (deviceId) mutation.mutate({ deviceId, purge });
    },
  };
}
