import { CheckIcon, CircleCheckIcon } from 'lucide-react';
import { useTranslation } from 'react-i18next';
import { Link } from 'react-router';
import { Button } from '@/components/ui/button';
import { Progress } from '@/components/ui/progress';
import { Spinner } from '@/components/ui/spinner';
import type { FirstSyncPhase } from '../_types/link';

export function DoneStep({
  profileName,
  phase,
  step,
  steps,
  profileTo,
}: {
  profileName: string;
  phase: FirstSyncPhase;
  step: number;
  steps: number;
  profileTo: string;
}) {
  const { t } = useTranslation();
  const label = { waiting: t('link.done.waiting'), connected: t('link.done.syncing'), synced: t('link.done.synced') }[phase];
  return (
    <section className="basis-form text-center md:rounded-2xl md:border md:border-border md:bg-card md:p-8">
      <div className="mx-auto flex size-14 items-center justify-center rounded-full bg-success/14 text-success">
        <CircleCheckIcon className="size-7" />
      </div>
      <h1 className="mt-4 text-title font-bold">{t('link.done.title', { name: profileName })}</h1>
      <p className="mt-1.5 text-muted-foreground">{t('link.done.description')}</p>
      <div className="mt-6 rounded-xl border border-border bg-popover px-4 py-3.5 text-left" role="status" aria-live="polite">
        <div className="flex items-center justify-between font-medium">
          <span className="flex items-center gap-2">
            {phase === 'synced' ? <CheckIcon className="size-4 text-success" /> : <Spinner className="text-muted-foreground" />}
            {t('link.done.firstSync')}
          </span>
          <span className="text-muted-foreground">{t('link.done.step', { step, total: steps })}</span>
        </div>
        <Progress value={(step * 100) / steps} size="sm" className="mt-2.5" />
        <div className="mt-2 text-caption text-muted-foreground">{label}</div>
      </div>
      <Button size="lg" className="mt-6 w-full" render={<Link to={profileTo} />} nativeButton={false}>
        {t('link.done.cta')}
      </Button>
    </section>
  );
}
