import type { Trophy } from '@openpsapp/contract/api';
import { formatDayMonth } from '@openpsapp/core';
import { useTranslation } from 'react-i18next';
import { GameCover } from '@/components/game-cover';

export function RecentTrophies({ trophies }: { trophies: Trophy[] }) {
  const { t, i18n } = useTranslation();
  if (trophies.length === 0) return null;
  return (
    <section className="max-md:hidden">
      <h2 className="text-xl font-bold">{t('game.recentlyEarned')}</h2>
      <div className="mt-3.5 grid grid-cols-fill-55 gap-3">
        {trophies.map((trophy) => (
          <div key={trophy.id} className="flex items-center gap-3 rounded-2xl border border-border bg-card p-3">
            <GameCover src={trophy.iconUrl} className="size-cover-md rounded-lg" />
            <div className="min-w-0">
              <div className="truncate text-body font-bold">{trophy.name}</div>
              <div className="text-caption text-muted-foreground">
                {t('game.recentMeta', { grade: t(`grade.${trophy.grade}`), date: formatDayMonth(trophy.earnedAt, i18n.language) })}
              </div>
            </div>
          </div>
        ))}
      </div>
    </section>
  );
}
