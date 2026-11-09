import { useState } from 'react';
import { useTranslation } from 'react-i18next';
import { combine, paths, post, useApi, type Game, type TrophySet } from '../api';
import { BackLink, ErrorBox, Loading } from '../components/ui';
import GameHeader from './GameHeader';
import TrophySetSection from './TrophySetSection';

export default function GamePage({ titleId }: { titleId: string }) {
  const { t } = useTranslation();
  const game = useApi<Game>(paths.game(titleId));
  const setId = game.data?.trophySetId ?? null;
  const set = useApi<TrophySet>(setId ? paths.trophySet(setId) : null);
  const [saving, setSaving] = useState(false);
  const status = combine(game, set);

  if (status.error) return <ErrorBox message={status.error} onRetry={status.reload} />;
  if (!game.data || (setId && !set.data)) return <Loading />;

  const toggleHidden = (g: Game) => {
    setSaving(true);
    post<Game>(paths.hide(g.titleId, !g.hidden))
      .then(game.reload)
      .catch(() => undefined)
      .then(() => setSaving(false));
  };

  const hideButton = (
    <button
      disabled={saving}
      onClick={() => toggleHidden(game.data!)}
      className="mt-3 px-4 py-2 rounded-lg border border-line text-sm text-muted hover:text-ink hover:bg-raised"
    >
      {game.data.hidden ? t('game.unhide') : t('game.hide')}
    </button>
  );

  return (
    <>
      <BackLink />
      <GameHeader game={game.data} set={set.data} actions={hideButton} />
      {setId ? (
        <TrophySetSection setId={setId} />
      ) : (
        <p className="mt-10 text-muted">{t('game.noTrophies')}</p>
      )}
    </>
  );
}
