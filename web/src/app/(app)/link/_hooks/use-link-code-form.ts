import { zodResolver } from '@hookform/resolvers/zod';
import type { PairingInspectResponse } from '@openpsapp/contract/pairing';
import { parseUserCode } from '@openpsapp/core';
import { useMutation } from '@tanstack/react-query';
import { useForm } from 'react-hook-form';
import { apiPost } from '@/lib/api';
import { codeError } from '../_lib/describe-link';
import { linkCodeSchema, type LinkCodeValues } from '../_schemas/link-code.schema';
import type { CodeError } from '../_types/link';

export function useLinkCodeForm({
  initialCode,
  onInspected,
}: {
  initialCode: string;
  onInspected: (code: string, result: PairingInspectResponse) => void;
}) {
  const form = useForm<LinkCodeValues>({ resolver: zodResolver(linkCodeSchema), defaultValues: { code: initialCode } });
  const inspect = useMutation({
    mutationFn: (code: string) => apiPost<PairingInspectResponse>('/devices/inspect', { userCode: code }),
    onSuccess: (result, code) => onInspected(code, result),
    onError: (error) => form.setError('root', { type: codeError(error) }),
  });
  const run = (values: LinkCodeValues) => {
    const code = parseUserCode(values.code);
    if (code) inspect.mutate(code);
  };
  const rootError = form.formState.errors.root?.type as CodeError | undefined;
  return {
    form,
    pending: inspect.isPending,
    rootError,
    submit: form.handleSubmit(run),
    inspect: (code: string) => {
      form.clearErrors();
      form.setValue('code', code);
      void form.handleSubmit(run)();
    },
    reset: () => {
      form.reset({ code: '' });
      form.setFocus('code');
    },
    invalidate: () => form.setError('root', { type: 'invalid' satisfies CodeError }),
  };
}
