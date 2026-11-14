import type { Trophy } from '@openpsapp/contract/api';
import { formatDate } from '@openpsapp/core';
import { cn } from 'cn';
import { useTranslation } from 'react-i18next';
import { GameCover } from '@/components/game-cover';
import { TrophyGlyph } from '@/components/trophy-glyph';
import { Button } from '@/components/ui/button';
import type { TrophyRowView } from '../_types/game';

const GRADE_TEXT: Record<Trophy['grade'], string> = {
  platinum: 'text-platinum-foreground',
  gold: 'text-gold',
  silver: 'text-silver-foreground',
  bronze: 'text-bronze-foreground',
  unknown: 'text-muted-foreground',
};

function TrophyIcon({ row }: { row: TrophyRowView }) {
  const box = 'flex size-cover-md shrink-0 items-center justify-center rounded-lg bg-muted md:size-cover-xl md:rounded-xl';
  if (row.masked) return <div className={cn(box, 'text-title font-extrabold text-muted-foreground')}>?</div>;
  if (row.earned && row.iconUrl) return <GameCover src={row.iconUrl} className="size-cover-md rounded-lg md:size-cover-xl md:rounded-xl" />;
  return (
    <div className={box}>
      <TrophyGlyph grade={row.grade} className="size-6 md:size-7" />
    </div>
  );
}

export function TrophyList({ rows, onReveal }: { rows: TrophyRowView[]; onReveal: (id: number) => void }) {
  const { t, i18n } = useTranslation();
  return (
    <ul className="grid grid-cols-1 gap-2 md:grid-cols-fill-115 md:gap-3">
      {rows.map((row) => {
        const grade = t(`grade.${row.grade}`);
        const when = row.earned ? formatDate(row.earnedAt, i18n.language) : t('game.locked');
        return (
          <li
            key={row.id}
            className="flex items-center gap-3 rounded-xl border border-border bg-card p-2.5 md:gap-4 md:rounded-2xl md:p-3.5"
          >
            <TrophyIcon row={row} />
            <div className="min-w-0 flex-1">
              <div className="truncate font-bold">{row.masked ? t('game.hiddenName') : row.name}</div>
              <div className="text-caption text-muted-foreground md:hidden">
                {row.earned ? `${grade} · ${when}` : t('game.lockedShort', { grade })}
              </div>
              <div className="line-clamp-2 text-sm text-muted-foreground max-md:hidden">
                {row.masked ? t('game.hiddenDescription') : row.description}
              </div>
            </div>
            {row.masked ? (
              <Button variant="outline" onClick={() => onReveal(row.id)} className="border-border font-semibold">
                {t('game.reveal')}
              </Button>
            ) : (
              <div className="text-right text-caption max-md:hidden">
                <div className={cn('font-semibold', GRADE_TEXT[row.grade])}>{grade}</div>
                <div className="text-muted-foreground">{when}</div>
              </div>
            )}
          </li>
        );
      })}
    </ul>
  );
}
