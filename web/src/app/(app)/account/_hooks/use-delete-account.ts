import { useClerk } from '@clerk/react';
import { useMutation, useQueryClient } from '@tanstack/react-query';
import { useState } from 'react';
import { useTranslation } from 'react-i18next';
import { toast } from 'sonner';
import { apiDelete } from '@/lib/api';
import { paths } from '@/lib/links';
import { deleteError } from '../_lib/describe-account';

export function useDeleteAccount() {
  const { t } = useTranslation();
  const clerk = useClerk();
  const queryClient = useQueryClient();
  const [open, setOpen] = useState(false);
  const mutation = useMutation({
    mutationFn: () => apiDelete('/me'),
    onSuccess: async () => {
      toast.success(t('account.delete.done'));
      setOpen(false);
      queryClient.clear();
      await clerk.signOut({ redirectUrl: paths.profile });
    },
    onError: (error) => toast.error(t(`account.delete.${deleteError(error)}`)),
  });
  return {
    open,
    pending: mutation.isPending,
    setOpen: (next: boolean) => {
      if (!mutation.isPending) setOpen(next);
    },
    confirm: () => mutation.mutate(),
  };
}
