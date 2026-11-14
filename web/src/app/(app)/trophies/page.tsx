import { useTranslation } from 'react-i18next';
import { ErrorState } from '@/components/error-state';
import { PageLoader } from '@/components/page-loader';
import { useEarnedTrophies } from '@/hooks/use-earned-trophies';
import { useGames } from '@/hooks/use-games';
import { useProfile } from '@/hooks/use-profile';
import { useTrophySets } from '@/hooks/use-trophy-sets';
import { paths } from '@/lib/links';
import { GradeTotals } from './_components/grade-totals';
import { SetCard } from './_components/set-card';
import { TrophiesToolbar } from './_components/trophies-toolbar';
import { TrophyTimeline } from './_components/trophy-timeline';
import { useTrophiesView } from './_hooks/use-trophies-view';
import { gradeTotals, SET_SORTS, setCards, timeline, TIMELINE_PREVIEW, visibleSets } from './_lib/describe-trophies';

const NO_ITEMS: never[] = [];

export default function TrophiesPage() {
  const { t } = useTranslation();
  const sets = useTrophySets();
  const games = useGames();
  const earned = useEarnedTrophies();
  const profile = useProfile();
  const view = useTrophiesView();

  if (sets.isError) return <ErrorState onRetry={() => void sets.refetch()} />;
  if (!sets.data) return <PageLoader />;

  const earnedList = earned.data ?? NO_ITEMS;
  const totals = profile.data?.totals.trophies;
  const cards = setCards(visibleSets(sets.data, view.sort, view.incompleteOnly), games.data ?? NO_ITEMS);

  return (
    <div className="mx-auto max-w-page px-4 pt-6 pb-16 sm:px-6 md:pt-9 lg:px-12">
      <div className="flex flex-wrap items-end justify-between gap-4">
        <div>
          <h1 className="text-heading font-bold">{t('trophies.title')}</h1>
          {totals && (
            <p className="mt-0.5 text-sm text-muted-foreground">
              {t('trophies.summary', {
                earned: totals.earned,
                total: totals.total,
                games: t('profile.games', { count: sets.data.length }),
              })}
            </p>
          )}
        </div>
        <TrophiesToolbar
          sorts={SET_SORTS}
          sort={view.sort}
          onSortChange={view.setSort}
          incompleteOnly={view.incompleteOnly}
          onIncompleteOnlyChange={view.setIncompleteOnly}
        />
      </div>
      <div className="mt-6">
        <GradeTotals totals={gradeTotals(sets.data)} />
      </div>
      <div className="mt-7 flex flex-wrap items-start gap-7">
        <div className="min-w-0 grow-999 basis-main">
          {cards.length === 0 ? (
            <p className="py-10 text-center text-muted-foreground">
              {sets.data.length === 0 ? t('trophies.empty') : t('trophies.emptyFilter')}
            </p>
          ) : (
            <div className="grid grid-cols-1 gap-4 sm:grid-cols-fill-75">
              {cards.map((card) => (
                <SetCard key={card.id} set={card} href={card.titleId ? paths.game(card.titleId) : null} />
              ))}
            </div>
          )}
        </div>
        <TrophyTimeline
          days={timeline(earnedList, view.fullTimeline ? null : TIMELINE_PREVIEW)}
          expandable={earnedList.length > TIMELINE_PREVIEW}
          expanded={view.fullTimeline}
          onToggle={view.toggleTimeline}
        />
      </div>
    </div>
  );
}
