import { formatDecimal, formatMonth, formatMonthName } from '@openpsapp/core';
import { ChevronLeftIcon, ChevronRightIcon } from 'lucide-react';
import { useTranslation } from 'react-i18next';
import { Link } from 'react-router';
import { GameCover } from '@/components/game-cover';
import { Button } from '@/components/ui/button';
import type { TopGame } from '../_types/profile';

export function MonthRecap({
  month,
  total,
  perWeek,
  top,
  canPrevious,
  canNext,
  onPrevious,
  onNext,
  gameHref,
}: {
  month: string;
  total: number;
  perWeek: number;
  top: TopGame | null;
  canPrevious: boolean;
  canNext: boolean;
  onPrevious: () => void;
  onNext: () => void;
  gameHref: (titleId: string) => string;
}) {
  const { t, i18n } = useTranslation();
  const label = 'text-caption font-semibold tracking-label text-muted-foreground uppercase';
  const topContent = top && (
    <>
      <GameCover src={top.iconUrl} className="size-cover-md rounded-lg" />
      <span className="min-w-0">
        <span className="block truncate font-bold">{top.name}</span>
        <span className="block text-sm text-muted-foreground">
          {t('profile.month.topGameCount', { count: top.count, month: formatMonthName(month, i18n.language) })}
        </span>
      </span>
    </>
  );
  return (
    <section className="rounded-3xl border border-border bg-card p-6">
      <div className="flex items-center justify-between gap-2">
        <Button
          variant="outline"
          size="icon-lg"
          className="rounded-full"
          disabled={!canPrevious}
          onClick={onPrevious}
          aria-label={t('profile.month.previous')}
        >
          <ChevronLeftIcon />
        </Button>
        <h2 className="text-lg font-bold">{formatMonth(month, i18n.language)}</h2>
        <Button
          variant="outline"
          size="icon-lg"
          className="rounded-full"
          disabled={!canNext}
          onClick={onNext}
          aria-label={t('profile.month.next')}
        >
          <ChevronRightIcon />
        </Button>
      </div>
      {total === 0 ? (
        <p className="mt-5 text-center text-muted-foreground">{t('profile.month.empty')}</p>
      ) : (
        <>
          <div className="mt-5 grid grid-cols-2 gap-4">
            <div>
              <div className={label}>{t('profile.month.trophies')}</div>
              <div className="text-3xl font-extrabold tabular-nums">{total}</div>
            </div>
            <div>
              <div className={label}>{t('profile.month.pace')}</div>
              <div className="text-3xl font-extrabold tabular-nums">
                {formatDecimal(perWeek, i18n.language)}
                <span className="text-body font-medium text-muted-foreground"> {t('profile.month.perWeek')}</span>
              </div>
            </div>
          </div>
          {top && (
            <div className="mt-5 border-t border-border pt-5">
              <div className={label}>{t('profile.month.topGame')}</div>
              {top.titleId ? (
                <Link to={gameHref(top.titleId)} className="mt-2.5 flex items-center gap-3 hover:opacity-90">
                  {topContent}
                </Link>
              ) : (
                <div className="mt-2.5 flex items-center gap-3">{topContent}</div>
              )}
            </div>
          )}
        </>
      )}
    </section>
  );
}
