import { useState } from 'react';
import { useTranslation } from 'react-i18next';
import { combine, paths, useApi, type Game, type Profile as ProfileData, type TrophySet } from '../api';
import { CardRow, Cover, ErrorBox, Loading, ProgressBar, Section, Stat, TrophyTotals } from '../components/ui';
import { formatDuration } from '../format';
import { useDate } from '../i18n/useLanguage';
import { href } from '../router';

export default function Profile() {
  const { t } = useTranslation();
  const date = useDate();
  const profile = useApi<ProfileData>(paths.profile);
  const games = useApi<Game[]>(paths.games);
  const sets = useApi<TrophySet[]>(paths.trophySets);
  const [showHidden, setShowHidden] = useState(false);
  const status = combine(profile, games);

  if (status.error) return <ErrorBox message={status.error} onRetry={status.reload} />;
  if (!profile.data || !games.data) return <Loading />;

  const { user, totals } = profile.data;
  const setsById = new Map((sets.data ?? []).map((s) => [s.id, s]));
  const hiddenCount = games.data.filter((g) => g.hidden).length;
  const listed = games.data.filter((g) => showHidden || !g.hidden);

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

      <div className="grid grid-cols-2 gap-4 mt-8">
        <Stat label={t('profile.playtime')}>{formatDuration(totals.playtimeSeconds)}</Stat>
        <TrophyTotals label={t('profile.trophies')} trophies={totals.trophies} />
      </div>

      <Section title={t('profile.recent')}>
        <ul>
          {listed.map((g) => {
            const set = g.trophySetId ? setsById.get(g.trophySetId) : undefined;
            const title = (
              <>
                {g.name}
                {g.hidden && <span className="ml-3 text-sm font-normal text-muted">{t('profile.hiddenTag')}</span>}
              </>
            );
            return (
              <CardRow key={g.titleId} to={href.game(g.titleId)} icon={<Cover src={g.iconUrl} size="w-24 h-24" />} title={title}>
                <div className="text-muted mt-1">
                  {t('profile.playedOn', { duration: formatDuration(g.playtimeSeconds), date: date(g.lastPlayedAt) })}
                </div>
                {set && (
                  <div className="mt-3">
                    <ProgressBar value={set.progress} />
                  </div>
                )}
              </CardRow>
            );
          })}
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
