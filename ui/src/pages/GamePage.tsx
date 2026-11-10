import { useState } from 'react';
import { useTranslation } from 'react-i18next';
import GameHeader from '@/components/game/GameHeader';
import TrophySetSection from '@/components/trophies/TrophySetSection';
import BackLink from '@/components/ui/BackLink';
import ErrorBox from '@/components/ui/ErrorBox';
import Loading from '@/components/ui/Loading';
import { combine, useApi } from '@/hooks/useApi';
import { endpoints, setGameHidden } from '@/services/api';
import type { Game } from '@/types/api';

export default function GamePage({ titleId }: { titleId: string }) {
  const { t } = useTranslation();
  const game = useApi(endpoints.game(titleId));
  const setId = game.data?.trophySetId ?? null;
  const set = useApi(setId ? endpoints.trophySet(setId) : null);
  const [saving, setSaving] = useState(false);
  const status = combine(game, set);

  if (status.error) return <ErrorBox message={status.error} onRetry={status.reload} />;
  if (!game.data || (setId && !set.data)) return <Loading />;

  const toggleHidden = (g: Game) => {
    setSaving(true);
    setGameHidden(g.titleId, !g.hidden)
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
