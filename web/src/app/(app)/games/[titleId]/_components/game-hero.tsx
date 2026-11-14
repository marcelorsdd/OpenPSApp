import type { Grade, GradeCount } from '@openpsapp/contract/api';
import { ChevronLeftIcon } from 'lucide-react';
import type { ReactNode } from 'react';
import { useTranslation } from 'react-i18next';
import { Link } from 'react-router';
import { GameCover } from '@/components/game-cover';
import { GradeCounts } from '@/components/grade-counts';
import { Button } from '@/components/ui/button';
import { Progress } from '@/components/ui/progress';

export function GameHero({
  name,
  iconUrl,
  heroUrl,
  summary,
  summaryShort,
  backTo,
  progress,
  counts,
}: {
  name: string;
  iconUrl: string | null;
  heroUrl: string | null;
  summary: string;
  summaryShort: string;
  backTo: string;
  progress: number | null;
  counts: Record<Grade, GradeCount> | null;
}) {
  const { t } = useTranslation();
  let stats: ReactNode = null;
  if (progress !== null && counts) {
    stats = (
      <div className="mt-4.5 flex flex-wrap items-center gap-5 max-md:hidden">
        <div className="flex grow basis-65 items-center gap-3">
          <Progress value={progress} className="flex-1 [&_[data-slot=progress-track]]:bg-foreground/18" />
          <span className="text-xl font-extrabold tabular-nums">{progress}%</span>
        </div>
        <GradeCounts counts={counts} />
      </div>
    );
  }
  return (
    <section className="relative overflow-hidden">
      {heroUrl && <img src={heroUrl} alt="" className="absolute inset-0 size-full object-cover object-y-25" />}
      <div
        aria-hidden="true"
        className="absolute inset-0 bg-linear-to-b from-background/10 via-background/55 to-background max-md:from-background/0 max-md:from-30%"
      />
      <div className="relative mx-auto max-w-page px-4 pt-3 pb-2 sm:px-6 md:pt-6 md:pb-9 lg:px-12">
        <Button
          variant="overlay"
          className="h-11 px-3.5 text-(length:--text-body) font-semibold max-md:size-11 max-md:px-0"
          render={<Link to={backTo} />}
          nativeButton={false}
        >
          <ChevronLeftIcon className="size-5" />
          <span className="max-md:sr-only">{t('common.back')}</span>
        </Button>
        <div className="mt-30 flex items-end gap-3.5 md:mt-hero-gap md:flex-wrap md:gap-7">
          <GameCover
            src={iconUrl}
            className="size-cover-2xl rounded-2xl shadow-ring-page md:size-cover-3xl md:rounded-4xl md:shadow-cover"
          />
          <div className="min-w-0 flex-1 md:basis-105">
            <h1 className="text-title font-extrabold md:text-display">{name}</h1>
            <p className="text-caption text-muted-foreground md:mt-2 md:text-lead md:text-soft-foreground">
              <span className="md:hidden">{summaryShort}</span>
              <span className="max-md:hidden">{summary}</span>
            </p>
            {stats}
          </div>
        </div>
      </div>
    </section>
  );
}
