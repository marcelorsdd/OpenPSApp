import { combine, paths, useApi, type Game, type TrophySet } from '../api';
import { BackLink, ErrorBox, Loading } from '../components/ui';
import GameHeader from './GameHeader';
import TrophySetSection from './TrophySetSection';

export default function SetPage({ setId }: { setId: string }) {
  const set = useApi<TrophySet>(paths.trophySet(setId));
  const titleId = set.data?.titleId ?? null;
  const game = useApi<Game>(titleId ? paths.game(titleId) : null);
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
