import { useTranslation } from 'react-i18next';
import type { PlatinumStatus } from '../_types/profile';

export function PlatinumCard({ status }: { status: PlatinumStatus }) {
  const { t } = useTranslation();
  const parts =
    status.count === 0
      ? [
          t('profile.platinums.none'),
          status.closest && t('profile.platinums.closest', { game: status.closest.name, progress: status.closest.progress }),
        ]
      : [t('profile.platinums.count', { count: status.count }), status.latest && t('profile.platinums.latest', { game: status.latest })];
  return (
    <section className="rounded-3xl border border-border bg-card px-6 py-5">
      <h2 className="font-bold">{t('profile.platinums.title')}</h2>
      <p className="mt-1.5 text-body text-muted-foreground">{parts.filter(Boolean).join(' ')}</p>
    </section>
  );
}
