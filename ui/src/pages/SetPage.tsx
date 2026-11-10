import GameHeader from '@/components/game/GameHeader';
import TrophySetSection from '@/components/trophies/TrophySetSection';
import BackLink from '@/components/ui/BackLink';
import ErrorBox from '@/components/ui/ErrorBox';
import Loading from '@/components/ui/Loading';
import { combine, useApi } from '@/hooks/useApi';
import { endpoints } from '@/services/api';

export default function SetPage({ setId }: { setId: string }) {
  const set = useApi(endpoints.trophySet(setId));
  const titleId = set.data?.titleId ?? null;
  const game = useApi(titleId ? endpoints.game(titleId) : null);
  const status = combine(set, game);

  if (status.error) return <ErrorBox message={status.error} onRetry={status.reload} />;
  if (!set.data || (titleId && !game.data)) return <Loading />;

  return (
    <>
      <BackLink />
      <GameHeader game={game.data} set={set.data} />
      <TrophySetSection setId={setId} />
    </>
  );
}
