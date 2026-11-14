import { formatDate, formatDayMonth, formatDuration } from '@openpsapp/core';
import { useTranslation } from 'react-i18next';
import { useLocation, useParams } from 'react-router';
import { ErrorState } from '@/components/error-state';
import { GradeCountsText } from '@/components/grade-counts';
import { PageLoader } from '@/components/page-loader';
import { Progress } from '@/components/ui/progress';
import { isApiError } from '@/lib/api';
import { paths } from '@/lib/links';
import { GameHero } from './_components/game-hero';
import { RecentTrophies } from './_components/recent-trophies';
import { TrophyList } from './_components/trophy-list';
import { TrophyToolbar } from './_components/trophy-toolbar';
import { useGame } from './_hooks/use-game';
import { useTrophyList } from './_hooks/use-trophy-list';
import { backTarget, recentTrophies, TROPHY_FILTERS, TROPHY_SORTS, trophyCounts, trophyRows } from './_lib/describe-game';

export default function GamePage() {
  const { t, i18n } = useTranslation();
  const { titleId = '' } = useParams();
  const location = useLocation();
  const { game, set, trophies, setId } = useGame(titleId);
  const list = useTrophyList();
  const backTo = backTarget(location.state, paths.profile);

  if (isApiError(game.error, 'not_found')) {
    return <p className="px-4 py-24 text-center text-muted-foreground">{t('game.notFound')}</p>;
  }
  if (game.isError || set.isError || trophies.isError) {
    return (
      <ErrorState
        onRetry={() => {
          void game.refetch();
          void set.refetch();
          void trophies.refetch();
        }}
      />
    );
  }
  if (!game.data || (setId && (!set.data || !trophies.data))) return <PageLoader />;

  const data = game.data;
  const duration = formatDuration(data.playtimeSeconds);
  const allTrophies = trophies.data ?? [];
  const counts = trophyCounts(allTrophies);
  const rows = trophyRows(allTrophies, list);

  return (
    <>
      <GameHero
        name={data.name}
        iconUrl={data.iconUrl}
        heroUrl={data.heroUrl}
        backTo={backTo}
        summary={
          data.lastPlayedAt
            ? t('game.summary', { duration, date: formatDate(data.lastPlayedAt, i18n.language) })
            : t('game.summaryNever', { duration })
        }
        summaryShort={
          data.lastPlayedAt
            ? t('game.summaryShort', { duration, date: formatDayMonth(data.lastPlayedAt, i18n.language) })
            : t('game.summaryNever', { duration })
        }
        progress={set.data?.progress ?? null}
        counts={set.data?.counts ?? null}
      />
      <div className="mx-auto flex max-w-page flex-col gap-4 px-4 pb-16 sm:px-6 md:gap-9 md:pt-2 lg:px-12">
        {set.data && (
          <div className="md:hidden">
            <div className="flex items-center gap-2.5">
              <Progress value={set.data.progress} size="sm" className="flex-1" />
              <span className="font-extrabold tabular-nums">{set.data.progress}%</span>
            </div>
            <GradeCountsText counts={set.data.counts} className="mt-2 text-caption" />
          </div>
        )}
        {setId ? (
          <>
            <RecentTrophies trophies={recentTrophies(allTrophies)} />
            <div className="flex flex-col gap-3 md:gap-4">
              <TrophyToolbar
                filters={TROPHY_FILTERS}
                filter={list.filter}
                onFilterChange={list.setFilter}
                counts={counts}
                sorts={TROPHY_SORTS}
                sort={list.sort}
                onSortChange={list.setSort}
                showHidden={list.showHidden}
                onShowHiddenChange={list.setShowHidden}
              />
              {rows.length === 0 ? (
                <p className="py-10 text-center text-muted-foreground">{t('game.emptyFilter')}</p>
              ) : (
                <TrophyList rows={rows} onReveal={list.reveal} />
              )}
            </div>
          </>
        ) : (
          <p className="py-10 text-muted-foreground">{t('game.noTrophies')}</p>
        )}
      </div>
    </>
  );
}
