import type { EarnedTrophy } from '@openpsapp/contract/api';
import { formatDayMonth } from '@openpsapp/core';
import { useTranslation } from 'react-i18next';
import { Link } from 'react-router';
import { GameCover } from '@/components/game-cover';
import { GradeBadge } from '@/components/grade-badge';

function Row({ item }: { item: EarnedTrophy }) {
  const { t, i18n } = useTranslation();
  const date = formatDayMonth(item.trophy.earnedAt, i18n.language);
  return (
    <>
      <GameCover src={item.trophy.iconUrl} className="size-11 rounded-md md:size-cover-lg md:rounded-lg" />
      <div className="min-w-0 flex-1">
        <div className="truncate font-bold">{item.trophy.name}</div>
        <div className="truncate text-caption text-muted-foreground md:text-sm">
          <span className="md:hidden">
            {t('profile.trophyMetaShort', { game: item.setName, grade: t(`grade.${item.trophy.grade}`), date })}
          </span>
          <span className="max-md:hidden">{t('profile.trophyMeta', { game: item.setName, date })}</span>
        </div>
      </div>
      <span className="max-md:hidden">
        <GradeBadge grade={item.trophy.grade} size="lg" />
      </span>
    </>
  );
}

export function LatestTrophies({ items, gameHref }: { items: EarnedTrophy[]; gameHref: (titleId: string) => string }) {
  const { t } = useTranslation();
  const row = 'flex items-center gap-3 rounded-xl border border-border bg-card p-2.5 md:gap-3.5 md:rounded-2xl md:p-3';
  return (
    <section>
      <h2 className="text-lg font-bold md:text-title">{t('profile.latestTrophies')}</h2>
      {items.length === 0 ? (
        <p className="mt-3 text-muted-foreground">{t('profile.noTrophiesYet')}</p>
      ) : (
        <ul className="mt-2.5 grid grid-cols-1 gap-2 md:mt-4 md:grid-cols-fill-80 md:gap-3">
          {items.map((item) => (
            <li key={`${item.setId}:${item.trophy.id}`}>
              {item.titleId ? (
                <Link to={gameHref(item.titleId)} className={`${row} transition-colors hover:bg-accent/40`}>
                  <Row item={item} />
                </Link>
              ) : (
                <div className={row}>
                  <Row item={item} />
                </div>
              )}
            </li>
          ))}
        </ul>
      )}
    </section>
  );
}
