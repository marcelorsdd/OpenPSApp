import { useState } from 'react';
import { useTranslation } from 'react-i18next';
import GameCard from '@/components/profile/GameCard';
import MonthCard from '@/components/profile/MonthCard';
import TrophyGlyph from '@/components/trophies/TrophyGlyph';
import TrophyTotals from '@/components/trophies/TrophyTotals';
import ErrorBox from '@/components/ui/ErrorBox';
import Loading from '@/components/ui/Loading';
import Section from '@/components/ui/Section';
import Stat from '@/components/ui/Stat';
import { combine, useApi } from '@/hooks/useApi';
import { endpoints } from '@/services/api';
import { formatDuration } from '@/utils/format';

export default function ProfilePage() {
  const { t } = useTranslation();
  const profile = useApi(endpoints.profile);
  const games = useApi(endpoints.games);
  const sets = useApi(endpoints.trophySets);
  const earned = useApi(endpoints.earnedTrophies());
  const [showHidden, setShowHidden] = useState(false);
  const status = combine(profile, games);

  if (status.error) return <ErrorBox message={status.error} onRetry={status.reload} />;
  if (!profile.data || !games.data) return <Loading />;

  const { user, totals } = profile.data;
  const setsById = new Map((sets.data ?? []).map((s) => [s.id, s]));
  const hiddenCount = games.data.filter((g) => g.hidden).length;
  const listed = games.data.filter((g) => showHidden || !g.hidden);
  const progress = earned.data && earned.data.length > 0 && sets.data ? { earned: earned.data, sets: sets.data } : null;

  return (
    <>
      <div className="flex items-center">
        <div className="w-20 h-20 rounded-full bg-accent flex items-center justify-center text-3xl font-bold">
          {(user?.name ?? '?').slice(0, 1).toUpperCase()}
        </div>
        <div className="ml-5">
          <h1 className="text-3xl font-bold">{user?.name ?? t('profile.fallbackName')}</h1>
          <p className="text-muted">{t('profile.games', { count: totals.games })}</p>
        </div>
      </div>

      <div className="grid grid-cols-2 sm:grid-cols-3 gap-4 mt-8">
        <Stat label={t('profile.playtime')}>{formatDuration(totals.playtimeSeconds)}</Stat>
        <TrophyTotals label={t('profile.trophies')} trophies={totals.trophies} />
        <Stat label={t('profile.platinums')}>
          <span className="flex items-center">
            <TrophyGlyph grade="platinum" className="w-7 h-7" />
            <span className="ml-2">{totals.trophies.platinum}</span>
          </span>
        </Stat>
      </div>

      <Section title={t('profile.recent')}>
        <ul>
          {listed.map((g) => (
            <GameCard key={g.titleId} game={g} set={g.trophySetId ? setsById.get(g.trophySetId) : undefined} />
          ))}
        </ul>
        {hiddenCount > 0 && (
          <button className="mt-2 px-4 py-2 rounded-lg text-muted hover:text-ink" onClick={() => setShowHidden((v) => !v)}>
            {showHidden ? t('profile.hideHidden') : t('profile.showHidden', { count: hiddenCount })}
          </button>
        )}
      </Section>

      {progress && <MonthCard earned={progress.earned} sets={progress.sets} />}
    </>
  );
}
