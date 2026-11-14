import type { Grade, GradeCount } from '@openpsapp/contract/api';
import { GRADES } from '@openpsapp/core';
import { cn } from 'cn';
import { useTranslation } from 'react-i18next';
import { TrophyGlyph } from '@/components/trophy-glyph';

export function GradeCounts({ counts, className }: { counts: Record<Grade, number> | Record<Grade, GradeCount>; className?: string }) {
  const { t } = useTranslation();
  return (
    <div className={cn('flex flex-wrap items-center gap-x-5 gap-y-2 font-semibold tabular-nums', className)}>
      {GRADES.map((grade) => {
        const value = counts[grade];
        return (
          <span key={grade} className="flex items-center gap-1.5">
            <TrophyGlyph grade={grade} label={t(`grade.${grade}`)} />
            {typeof value === 'number' ? value : `${value.earned}/${value.total}`}
          </span>
        );
      })}
    </div>
  );
}

export function GradeCountsText({ counts, className }: { counts: Record<Grade, number> | Record<Grade, GradeCount>; className?: string }) {
  const { t } = useTranslation();
  const color: Record<Grade, string> = {
    platinum: 'text-platinum-foreground',
    gold: 'text-gold',
    silver: 'text-silver-foreground',
    bronze: 'text-bronze-foreground',
  };
  return (
    <div className={cn('flex justify-between gap-2 font-semibold tabular-nums', className)}>
      {GRADES.map((grade) => {
        const value = counts[grade];
        return (
          <span key={grade} className={color[grade]}>
            {t(`grade.${grade}`)} {typeof value === 'number' ? value : `${value.earned}/${value.total}`}
          </span>
        );
      })}
    </div>
  );
}
