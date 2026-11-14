import { useTranslation } from 'react-i18next';
import { Link } from 'react-router';
import { ConsoleTile } from '@/components/console-tile';
import { NumberedSteps } from '@/components/numbered-steps';
import { Button } from '@/components/ui/button';

export function NoConsoleCard({ linkTo }: { linkTo: string }) {
  const { t } = useTranslation();
  return (
    <section className="flex flex-col items-center rounded-2xl border border-dashed border-input bg-card px-5 py-10 text-center md:px-8">
      <ConsoleTile />
      <h2 className="mt-4 text-lg font-semibold">{t('account.empty.title')}</h2>
      <p className="mt-1.5 max-w-md text-muted-foreground">{t('account.empty.description')}</p>
      <NumberedSteps className="mt-6 w-full max-w-md" steps={[t('account.steps.one'), t('account.steps.two'), t('account.steps.three')]} />
      <Button size="lg" className="mt-6" render={<Link to={linkTo} />} nativeButton={false}>
        {t('account.empty.cta')}
      </Button>
    </section>
  );
}
