import { formatDayName } from '@openpsapp/core';
import { useTranslation } from 'react-i18next';
import { GameCover } from '@/components/game-cover';
import { TrophyGlyph } from '@/components/trophy-glyph';
import { Button } from '@/components/ui/button';
import type { TimelineDay } from '../_types/trophies';

export function TrophyTimeline({
  days,
  expandable,
  expanded,
  onToggle,
}: {
  days: TimelineDay[];
  expandable: boolean;
  expanded: boolean;
  onToggle: () => void;
}) {
  const { t, i18n } = useTranslation();
  return (
    <aside className="min-w-0 grow basis-75 rounded-2xl border border-border bg-card p-5">
      <h2 className="text-body font-semibold">{t('trophies.timeline')}</h2>
      {days.length === 0 && <p className="mt-3 text-muted-foreground">{t('trophies.timelineEmpty')}</p>}
      {days.map((day) => (
        <div key={day.day} className="mt-4">
          <div className="text-xs font-semibold tracking-label text-muted-foreground uppercase">
            {t('trophies.timelineDay', { day: formatDayName(day.day, i18n.language), count: day.count })}
          </div>
          <ul className="mt-2 flex flex-col gap-2.5 border-l-2 border-border pl-3.5">
            {day.items.map((item) => (
              <li key={`${item.setId}:${item.trophy.id}`} className="flex items-center gap-2.5">
                {item.trophy.iconUrl ? (
                  <GameCover src={item.trophy.iconUrl} className="size-cover-sm rounded-md" />
                ) : (
                  <span className="flex size-cover-sm shrink-0 items-center justify-center rounded-md bg-muted">
                    <TrophyGlyph grade={item.trophy.grade} />
                  </span>
                )}
                <div className="min-w-0">
                  <div className="truncate font-medium">{item.trophy.name}</div>
                  <div className="truncate text-xs text-muted-foreground">
                    {t('trophies.timelineItem', { game: item.setName, grade: t(`grade.${item.trophy.grade}`) })}
                  </div>
                </div>
              </li>
            ))}
          </ul>
        </div>
      ))}
      {expandable && (
        <Button variant="link" className="mt-4 font-medium" onClick={onToggle}>
          {expanded ? t('trophies.timelineLess') : t('trophies.timelineAll')}
        </Button>
      )}
    </aside>
  );
}
