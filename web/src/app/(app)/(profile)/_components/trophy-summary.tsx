import { cn } from 'cn';
import { useTranslation } from 'react-i18next';
import { GradeCounts, GradeCountsText } from '@/components/grade-counts';
import { Progress } from '@/components/ui/progress';
import type { TrophyTotals } from '../_types/profile';

const percent = (totals: TrophyTotals) => (totals.total ? Math.round((totals.earned * 100) / totals.total) : 0);

export function TrophySummary({ totals, className }: { totals: TrophyTotals; className?: string }) {
  const { t } = useTranslation();
  const value = percent(totals);
  return (
    <div className={cn('min-w-75 basis-aside rounded-3xl border border-border bg-card/85 px-6 py-5', className)}>
      <div className="flex items-baseline justify-between gap-3">
        <span className="text-caption font-semibold tracking-label text-muted-foreground uppercase">{t('profile.trophies')}</span>
        <span className="text-sm text-muted-foreground">{value}%</span>
      </div>
      <div className="mt-1 text-3xl font-extrabold tabular-nums">
        {totals.earned}{' '}
        <span className="text-lg font-medium text-muted-foreground">{t('profile.trophiesOf', { total: totals.total })}</span>
      </div>
      <Progress value={value} className="mt-3" />
      <GradeCounts counts={totals} className="mt-4" />
    </div>
  );
}

export function TrophySummaryCompact({ totals, className }: { totals: TrophyTotals; className?: string }) {
  const { t } = useTranslation();
  const value = percent(totals);
  return (
    <div className={cn('rounded-2xl border border-border bg-card px-4 py-3.5', className)}>
      <div className="flex items-baseline justify-between">
        <span className="text-title font-extrabold tabular-nums">
          {totals.earned}{' '}
          <span className="text-sm font-medium text-muted-foreground">{t('profile.trophiesOfLong', { total: totals.total })}</span>
        </span>
        <span className="text-sm text-muted-foreground">{value}%</span>
      </div>
      <Progress value={value} size="sm" className="mt-2" />
      <GradeCountsText counts={totals} className="mt-2.5 text-sm" />
    </div>
  );
}
