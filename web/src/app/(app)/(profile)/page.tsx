import { formatDuration } from '@openpsapp/core';
import { useState } from 'react';
import { useTranslation } from 'react-i18next';
import { ErrorState } from '@/components/error-state';
import { PageLoader } from '@/components/page-loader';
import { useDevices } from '@/hooks/use-devices';
import { useEarnedTrophies } from '@/hooks/use-earned-trophies';
import { useGames } from '@/hooks/use-games';
import { useProfile } from '@/hooks/use-profile';
import { useTrophySets } from '@/hooks/use-trophy-sets';
import { paths } from '@/lib/links';
import { describeSync } from '../_lib/sync-status';
import { LatestTrophies } from './_components/latest-trophies';
import { MonthRecap } from './_components/month-recap';
import { PlatinumCard } from './_components/platinum-card';
import { ProfileEmpty } from './_components/profile-empty';
import { ProfileHero } from './_components/profile-hero';
import { RecentGames } from './_components/recent-games';
import { TrophySummary, TrophySummaryCompact } from './_components/trophy-summary';
import { useMonthRecap } from './_hooks/use-month-recap';
import { heroImage, LATEST_TROPHIES, platinumStatus, profileGames, RECENT_GAMES } from './_lib/describe-profile';

const NO_ITEMS: never[] = [];

export default function ProfilePage() {
  const { t } = useTranslation();
  const profile = useProfile();
  const games = useGames();
  const sets = useTrophySets();
  const earned = useEarnedTrophies();
  const devices = useDevices();
  const [expanded, setExpanded] = useState(false);
  const recap = useMonthRecap(earned.data ?? NO_ITEMS, games.data ?? NO_ITEMS, sets.data ?? NO_ITEMS);

  if (profile.isError || games.isError) {
    return (
      <ErrorState
        onRetry={() => {
          void profile.refetch();
          void games.refetch();
        }}
      />
    );
  }
  if (!profile.data || !games.data) return <PageLoader />;

  const list = profileGames(games.data, sets.data ?? NO_ITEMS);
  if (list.length === 0) {
    const sync = describeSync(devices.data);
    return (
      <div className="px-4 sm:px-6">
        <ProfileEmpty waitingFor={sync.kind === 'waiting' ? sync.name : null} linkTo={paths.link} />
      </div>
    );
  }

  const { totals, user } = profile.data;
  const name = user?.name ?? t('profile.fallbackName');
  const gamesLabel = t('profile.games', { count: totals.games });
  const playtime = formatDuration(totals.playtimeSeconds);

  return (
    <>
      <ProfileHero
        name={name}
        initial={name.slice(0, 1).toUpperCase()}
        summary={t('profile.summary', { games: gamesLabel, playtime })}
        summaryShort={t('profile.summaryShort', { games: gamesLabel, playtime })}
        heroUrl={heroImage(list)}
        aside={<TrophySummary totals={totals.trophies} className="max-md:hidden" />}
      />
      <div className="mx-auto flex max-w-page flex-wrap items-start gap-5 px-4 pb-16 sm:px-6 md:gap-10 md:pt-2 lg:px-12">
        <TrophySummaryCompact totals={totals.trophies} className="w-full md:hidden" />
        <div className="flex min-w-0 grow-999 basis-main flex-col gap-5 md:gap-10">
          <RecentGames
            games={expanded ? list : list.slice(0, RECENT_GAMES)}
            canExpand={list.length > RECENT_GAMES}
            expanded={expanded}
            onToggle={() => setExpanded((value) => !value)}
            gameHref={paths.game}
          />
          <LatestTrophies items={(earned.data ?? NO_ITEMS).slice(0, LATEST_TROPHIES)} gameHref={paths.game} />
        </div>
        <aside className="flex min-w-0 grow basis-80 flex-col gap-4">
          <MonthRecap
            month={recap.month}
            total={recap.total}
            perWeek={recap.perWeek}
            top={recap.top}
            canPrevious={recap.canPrevious}
            canNext={recap.canNext}
            onPrevious={recap.previous}
            onNext={recap.next}
            gameHref={paths.game}
          />
          <PlatinumCard status={platinumStatus(sets.data ?? NO_ITEMS, earned.data ?? NO_ITEMS)} />
        </aside>
      </div>
    </>
  );
}
