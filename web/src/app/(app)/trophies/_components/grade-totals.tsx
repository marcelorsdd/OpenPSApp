import { useTranslation } from 'react-i18next';
import { TrophyGlyph } from '@/components/trophy-glyph';
import type { GradeTotal } from '../_types/trophies';

export function GradeTotals({ totals }: { totals: GradeTotal[] }) {
  const { t } = useTranslation();
  return (
    <div className="grid grid-cols-2 gap-3 md:grid-cols-4">
      {totals.map((total) => (
        <div key={total.grade} className="flex items-center gap-3 rounded-xl border border-border bg-card px-4 py-3.5">
          <TrophyGlyph grade={total.grade} className="size-7" />
          <div>
            <div className="text-xs text-muted-foreground">{t(`grade.${total.grade}`)}</div>
            <div className="text-xl font-bold tabular-nums">
              {total.earned}{' '}
              <span className="text-caption font-medium text-muted-foreground">{t('trophies.gradeOf', { total: total.total })}</span>
            </div>
          </div>
        </div>
      ))}
    </div>
  );
}
