import { useTranslation } from 'react-i18next';
import { Link } from 'react-router';
import { ConsoleTile } from '@/components/console-tile';
import { Button } from '@/components/ui/button';

export function ProfileEmpty({ waitingFor, linkTo }: { waitingFor: string | null; linkTo: string }) {
  const { t } = useTranslation();
  return (
    <section className="mx-auto mt-10 flex max-w-narrow flex-col items-center rounded-2xl border border-dashed border-input bg-card px-8 py-10 text-center">
      <ConsoleTile />
      <h1 className="mt-4 text-lg font-semibold">{waitingFor ? t('profile.waiting.title') : t('profile.empty.title')}</h1>
      <p className="mt-1.5 max-w-form text-muted-foreground">
        {waitingFor ? t('profile.waiting.description', { name: waitingFor }) : t('profile.empty.description')}
      </p>
      {!waitingFor && (
        <Button size="lg" className="mt-6" render={<Link to={linkTo} />} nativeButton={false}>
          {t('profile.empty.cta')}
        </Button>
      )}
    </section>
  );
}
