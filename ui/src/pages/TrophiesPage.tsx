import { useTranslation } from 'react-i18next';
import GradeCounts from '@/components/trophies/GradeCounts';
import TrophyTimeline from '@/components/trophies/TrophyTimeline';
import TrophyTotals from '@/components/trophies/TrophyTotals';
import CardRow from '@/components/ui/CardRow';
import Cover from '@/components/ui/Cover';
import ErrorBox from '@/components/ui/ErrorBox';
import Loading from '@/components/ui/Loading';
import Pill from '@/components/ui/Pill';
import ProgressBar from '@/components/ui/ProgressBar';
import { useApi } from '@/hooks/useApi';
import { useDate } from '@/hooks/useDate';
import { useStoredState } from '@/hooks/useStoredState';
import { endpoints } from '@/services/api';
import { href } from '@/utils/router';

export default function TrophiesPage() {
  const { t } = useTranslation();
  const date = useDate();
  const [stored, setView] = useStoredState<'games' | 'timeline'>('trophies.view', 'games');
  const view = stored === 'timeline' ? 'timeline' : 'games';
  const sets = useApi(endpoints.trophySets);
  const profile = useApi(endpoints.profile);

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

      <div className="flex flex-wrap mt-8 -mx-1">
        <Pill active={view === 'games'} onClick={() => setView('games')}>
          {t('trophies.byGame')}
        </Pill>
        <Pill active={view === 'timeline'} onClick={() => setView('timeline')}>
          {t('trophies.timeline')}
        </Pill>
      </div>

      {view === 'timeline' ? (
        <TrophyTimeline />
      ) : (
        <>
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
      )}
    </>
  );
}
