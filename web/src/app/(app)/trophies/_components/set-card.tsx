import { formatDate, GRADES } from '@openpsapp/core';
import { useTranslation } from 'react-i18next';
import { Link } from 'react-router';
import { GameCover } from '@/components/game-cover';
import { GradeBadge } from '@/components/grade-badge';
import { Progress } from '@/components/ui/progress';
import type { SetCardView } from '../_types/trophies';

function Content({ set }: { set: SetCardView }) {
  const { t, i18n } = useTranslation();
  return (
    <>
      <div className="relative h-33 bg-muted">
        {set.heroUrl ? (
          <img src={set.heroUrl} alt="" loading="lazy" className="size-full object-cover object-y-30" />
        ) : (
          <div className="size-full bg-brand-tile" />
        )}
        <div aria-hidden="true" className="absolute inset-0 bg-linear-to-b from-card/0 from-30% to-card" />
        <GameCover src={set.iconUrl} className="absolute -bottom-4.5 left-4 size-cover-lg rounded-lg shadow-ring-card" />
      </div>
      <div className="px-4 pt-6.5 pb-4">
        <div className="flex items-baseline justify-between gap-2">
          <div className="min-w-0 truncate text-base font-semibold">{set.name}</div>
          <div className="font-bold tabular-nums">{set.progress}%</div>
        </div>
        <Progress value={set.progress} size="sm" className="mt-2" />
        <div className="mt-3 flex flex-wrap gap-1.5">
          {GRADES.map((grade) => (
            <GradeBadge key={grade} grade={grade}>
              {t('trophies.gradeCount', { grade: t(`grade.${grade}`), earned: set.counts[grade].earned, total: set.counts[grade].total })}
            </GradeBadge>
          ))}
        </div>
        <div className="mt-3 text-caption text-muted-foreground">
          {set.lastEarnedAt
            ? t('trophies.setMeta', { earned: set.earned, total: set.total, date: formatDate(set.lastEarnedAt, i18n.language) })
            : t('trophies.setMetaNone', { earned: set.earned, total: set.total })}
        </div>
      </div>
    </>
  );
}

export function SetCard({ set, href }: { set: SetCardView; href: string | null }) {
  const frame = 'block overflow-hidden rounded-2xl border border-border bg-card';
  return href ? (
    <Link to={href} state={{ from: '/trophies' }} className={`${frame} transition-colors hover:bg-accent/40`}>
      <Content set={set} />
    </Link>
  ) : (
    <div className={frame}>
      <Content set={set} />
    </div>
  );
}
