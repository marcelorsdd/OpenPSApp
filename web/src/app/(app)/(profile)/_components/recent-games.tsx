import { formatDayMonth, formatDuration } from '@openpsapp/core';
import { useTranslation } from 'react-i18next';
import { Button } from '@/components/ui/button';
import type { ProfileGame } from '../_types/profile';
import { GameCard } from './game-card';

export function RecentGames({
  games,
  canExpand,
  expanded,
  onToggle,
  gameHref,
}: {
  games: ProfileGame[];
  canExpand: boolean;
  expanded: boolean;
  onToggle: () => void;
  gameHref: (titleId: string) => string;
}) {
  const { t, i18n } = useTranslation();
  return (
    <section>
      <div className="flex items-baseline justify-between gap-4">
        <h2 className="text-lg font-bold md:text-title">{t('profile.recent')}</h2>
        {canExpand && (
          <Button variant="link" className="text-(length:--text-body) font-semibold" onClick={onToggle}>
            {expanded ? t('profile.showLess') : t('profile.seeAll')}
          </Button>
        )}
      </div>
      <div className="scrollbar-none -mr-4 mt-2.5 flex snap-x gap-3 overflow-x-auto pr-4 md:mt-4 md:mr-0 md:grid md:grid-cols-fill-55 md:gap-4 md:overflow-visible md:pr-0">
        {games.map((game) => {
          const duration = formatDuration(game.playtimeSeconds);
          return (
            <GameCard
              key={game.titleId}
              game={game}
              href={gameHref(game.titleId)}
              meta={
                game.lastPlayedAt
                  ? t('profile.playedOn', { duration, date: formatDayMonth(game.lastPlayedAt, i18n.language) })
                  : t('profile.notPlayed', { duration })
              }
              metaShort={game.set ? t('profile.cardShort', { duration, progress: game.set.progress }) : duration}
              progressLabel={game.set ? t('profile.setProgress', { earned: game.set.earned, total: game.set.total }) : null}
              noTrophiesLabel={t('profile.noTrophies')}
            />
          );
        })}
      </div>
    </section>
  );
}
