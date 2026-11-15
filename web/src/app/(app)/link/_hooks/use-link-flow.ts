import type { DeviceInfo, PairingConfirmResponse } from '@openpsapp/contract/pairing';
import { useMutation, useQueryClient } from '@tanstack/react-query';
import { useEffect, useRef, useState } from 'react';
import { useTranslation } from 'react-i18next';
import { useLocation, useNavigate } from 'react-router';
import { toast } from 'sonner';
import { api, apiPost } from '@/lib/api';
import { queryKeys } from '@/lib/query-keys';
import { codeError, initialCode, linkCodeParam, shouldInspectOnLoad } from '../_lib/describe-link';
import { readScannedCode } from '../_lib/scanned-code';
import type { LinkStep } from '../_types/link';
import { useLinkCodeForm } from './use-link-code-form';

export function useLinkFlow() {
  const { t } = useTranslation();
  const queryClient = useQueryClient();
  const location = useLocation();
  const navigate = useNavigate();
  const [start] = useState(() => initialCode(linkCodeParam(location.search, location.hash)));
  const inspectedOnLoad = useRef(false);
  const [step, setStep] = useState<LinkStep>({ kind: 'code' });
  const code = useLinkCodeForm({
    initialCode: start,
    onInspected: (userCode, result) =>
      setStep({ kind: 'confirm', code: userCode, profileName: result.profileName, replaces: result.replaces }),
  });

  const confirm = useMutation({
    mutationFn: async (userCode: string) => {
      const before = await queryClient.fetchQuery({ queryKey: queryKeys.devices, queryFn: () => api<DeviceInfo[]>('/devices') });
      const result = await apiPost<PairingConfirmResponse>('/devices/confirm', { userCode });
      return { result, previousDeviceIds: before.map((device) => device.id) };
    },
    onSuccess: ({ result, previousDeviceIds }) => {
      toast.success(t('link.done.toastTitle'), { description: t('link.done.toastDescription') });
      setStep({ kind: 'done', profileName: result.profileName, previousDeviceIds });
      if (location.search || location.hash) void navigate({ pathname: location.pathname }, { replace: true });
    },
    onError: (error) => {
      setStep({ kind: 'code' });
      code.form.setError('root', { type: codeError(error) });
    },
  });

  useEffect(() => {
    if (inspectedOnLoad.current || !shouldInspectOnLoad(start)) return;
    inspectedOnLoad.current = true;
    code.inspect(start);
  });

  return {
    step,
    code,
    confirming: confirm.isPending,
    confirm: () => {
      if (step.kind === 'confirm') confirm.mutate(step.code);
    },
    typeCode: () => {
      code.form.clearErrors();
      setStep({ kind: 'code' });
    },
    otherCode: () => {
      setStep({ kind: 'code' });
      code.reset();
    },
    scan: () => {
      code.form.clearErrors();
      setStep({ kind: 'scan' });
    },
    acceptScan: (value: string) => {
      const scanned = readScannedCode(value, window.location.origin);
      if (!scanned) return false;
      setStep({ kind: 'code' });
      code.inspect(scanned);
      return true;
    },
  };
}
