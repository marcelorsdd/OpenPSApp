import { useState } from 'react';
import { useTranslation } from 'react-i18next';
import { combine, paths, useApi, type EarnedTrophy, type Game, type Profile as ProfileData, type TrophySet } from '../api';
import { ErrorBox, Loading, Section, Stat, TrophyGlyph, TrophyTotals } from '../components/ui';
import { formatDuration } from '../format';
import GameCard from './GameCard';
import Milestones from './Milestones';
import MonthCard from './MonthCard';
import RecentTrophies from './RecentTrophies';
import StatsCard from './StatsCard';

const RECENT = 6;

export default function Profile() {
  const { t } = useTranslation();
  const profile = useApi<ProfileData>(paths.profile);
  const games = useApi<Game[]>(paths.games);
  const sets = useApi<TrophySet[]>(paths.trophySets);
  const earned = useApi<EarnedTrophy[]>(paths.earnedTrophies());
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

      {progress && <MonthCard earned={progress.earned} sets={progress.sets} />}
      {progress && <Milestones earned={progress.earned} sets={progress.sets} />}
      {sets.data && <StatsCard games={games.data} sets={sets.data} />}
      {earned.data && <RecentTrophies items={earned.data.slice(0, RECENT)} />}

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
    </>
  );
}
