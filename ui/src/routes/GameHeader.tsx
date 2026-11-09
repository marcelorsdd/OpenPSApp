import type { ReactNode } from 'react';
import type { Game, TrophySet } from '../api';
import { Cover, GradeCounts, ProgressBar } from '../components/ui';
import { formatDate, formatDuration } from '../format';

export default function GameHeader({ game, set, actions }: { game?: Game | null; set?: TrophySet | null; actions?: ReactNode }) {
  return (
    <div className="flex items-start">
      <Cover src={game?.iconUrl ?? set?.iconUrl ?? null} size="w-40 h-40" />
      <div className="ml-6 flex-1 min-w-0">
        <h1 className="text-3xl font-bold">{game?.name ?? set?.name}</h1>
        {game && (
          <p className="text-muted mt-2">
            {formatDuration(game.playtimeSeconds)} jogados · último jogo em {formatDate(game.lastPlayedAt)}
          </p>
        )}
        {actions}
        {set && (
          <div className="mt-5">
            <ProgressBar value={set.progress} large />
            <div className="mt-3">
              <GradeCounts counts={set.counts} />
            </div>
          </div>
        )}
      </div>
    </div>
  );
}
