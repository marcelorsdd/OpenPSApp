import { InfoIcon } from 'lucide-react';
import { useTranslation } from 'react-i18next';
import { Alert, AlertDescription, AlertTitle } from '@/components/ui/alert';
import { Button } from '@/components/ui/button';

export function LandingHero({ linkNotice, whatIsSentUrl, onSignIn }: { linkNotice: boolean; whatIsSentUrl: string; onSignIn: () => void }) {
  const { t } = useTranslation();
  const steps = [t('landing.steps.one'), t('landing.steps.two'), t('landing.steps.three')];
  return (
    <div className="min-w-0 grow basis-110">
      {linkNotice && (
        <Alert className="mb-8">
          <InfoIcon />
          <AlertTitle>{t('landing.linkNotice.title')}</AlertTitle>
          <AlertDescription>{t('landing.linkNotice.description')}</AlertDescription>
        </Alert>
      )}
      <h1 className="text-hero font-extrabold text-balance">{t('landing.title')}</h1>
      <p className="mt-5 max-w-lg text-lg text-soft-foreground sm:text-xl">{t('landing.lead')}</p>
      <div className="mt-8 flex flex-wrap gap-3">
        <Button size="xl" onClick={onSignIn}>
          {t('landing.signIn')}
        </Button>
        <Button size="xl" variant="outline" render={<a href={whatIsSentUrl} target="_blank" rel="noreferrer" />} nativeButton={false}>
          {t('landing.whatIsSent')}
        </Button>
      </div>
      <ol aria-label={t('landing.steps.label')} className="mt-11 grid gap-4 sm:grid-cols-3">
        {steps.map((step, index) => (
          <li key={step} className="rounded-2xl border border-border bg-card p-4">
            <span className="flex size-7 items-center justify-center rounded-full bg-muted text-sm font-bold">{index + 1}</span>
            <p className="mt-2.5 font-semibold">{step}</p>
          </li>
        ))}
      </ol>
    </div>
  );
}
