import { useTranslation } from 'react-i18next';
import { paths, useApi, type Profile, type TrophySet } from '../api';
import { CardRow, Cover, ErrorBox, GradeCounts, Loading, ProgressBar, TrophyTotals } from '../components/ui';
import { useDate } from '../i18n/useLanguage';
import { href } from '../router';

export default function Trophies() {
  const { t } = useTranslation();
  const date = useDate();
  const sets = useApi<TrophySet[]>(paths.trophySets);
  const profile = useApi<Profile>(paths.profile);

  if (sets.error) return <ErrorBox message={sets.error} onRetry={sets.reload} />;
  if (!sets.data) return <Loading />;

  return (
    <>
      <h1 className="text-3xl font-bold">{t('trophies.title')}</h1>
      {profile.data && (
        <div className="mt-6">
          <TrophyTotals label={t('trophies.earned')} trophies={profile.data.totals.trophies} />
        </div>
      )}

      <ul className="mt-8">
        {sets.data.map((s) => {
          const aside = (
            <div className="text-sm text-muted hidden sm:block">
              <div>
                {s.earned}/{s.total}
              </div>
              <div className="mt-1">{s.lastEarnedAt ? date(s.lastEarnedAt) : t('trophies.noneYet')}</div>
            </div>
          );
          return (
            <CardRow key={s.id} to={href.set(s.id)} icon={<Cover src={s.iconUrl} size="w-24 h-24" />} title={s.name} aside={aside}>
              <div className="mt-2">
                <GradeCounts counts={s.counts} />
              </div>
              <div className="mt-3">
                <ProgressBar value={s.progress} />
              </div>
            </CardRow>
          );
        })}
      </ul>
      {sets.data.length === 0 && <p className="mt-8 text-muted">{t('trophies.empty')}</p>}
    </>
  );
}
