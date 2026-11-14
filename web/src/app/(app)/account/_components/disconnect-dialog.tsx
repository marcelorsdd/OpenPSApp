import { useRef } from 'react';
import { useTranslation } from 'react-i18next';
import {
  AlertDialog,
  AlertDialogAction,
  AlertDialogCancel,
  AlertDialogContent,
  AlertDialogDescription,
  AlertDialogFooter,
  AlertDialogHeader,
  AlertDialogTitle,
} from '@/components/ui/alert-dialog';
import { Checkbox } from '@/components/ui/checkbox';
import { Spinner } from '@/components/ui/spinner';

export function DisconnectDialog({
  open,
  onOpenChange,
  name,
  purge,
  onPurgeChange,
  pending,
  onConfirm,
}: {
  open: boolean;
  onOpenChange: (open: boolean) => void;
  name: string;
  purge: boolean;
  onPurgeChange: (purge: boolean) => void;
  pending: boolean;
  onConfirm: () => void;
}) {
  const { t } = useTranslation();
  const cancel = useRef<HTMLButtonElement>(null);
  return (
    <AlertDialog open={open} onOpenChange={onOpenChange}>
      <AlertDialogContent initialFocus={cancel}>
        <AlertDialogHeader>
          <AlertDialogTitle>{t('account.disconnect.title')}</AlertDialogTitle>
          <AlertDialogDescription>{t('account.disconnect.description', { name })}</AlertDialogDescription>
        </AlertDialogHeader>
        <label className="flex cursor-pointer items-start gap-3 rounded-lg border border-border p-3 max-md:rounded-xl max-md:p-3.5">
          <Checkbox checked={purge} onCheckedChange={(checked) => onPurgeChange(checked === true)} className="mt-0.5" disabled={pending} />
          <span>
            <span className="block font-medium">{t('account.disconnect.purge')}</span>
            <span className="block text-caption text-muted-foreground">{t('account.disconnect.purgeHint')}</span>
          </span>
        </label>
        <AlertDialogFooter>
          <AlertDialogCancel ref={cancel} disabled={pending}>
            {t('common.cancel')}
          </AlertDialogCancel>
          <AlertDialogAction variant="destructive" onClick={onConfirm} disabled={pending}>
            {pending && <Spinner />}
            {t('account.disconnect.confirm')}
          </AlertDialogAction>
        </AlertDialogFooter>
      </AlertDialogContent>
    </AlertDialog>
  );
}
