import type { ReactNode } from 'react';
import { useTranslation } from 'react-i18next';
import type { Game, TrophySet } from '../api';
import { Cover, GradeCounts } from '../components/ui';
import { formatDuration } from '../format';
import { isComplete } from '../progress';
import { href } from '../router';

export default function GameCard({ game, set }: { game: Game; set?: TrophySet }) {
  const { t } = useTranslation();
  return (
    <li className="mb-3">
      <a href={href.game(game.titleId)} className="block rounded-2xl bg-surface border border-line hover:bg-raised">
        <div className="flex items-center p-4">
          <Cover src={game.iconUrl} size="w-24 h-24" />
          <div className="ml-5 flex-1 min-w-0">
            <div className="text-lg font-semibold truncate">
              {game.name}
              {game.hidden && <span className="ml-3 text-sm font-normal text-muted">{t('profile.hiddenTag')}</span>}
            </div>
            <div className="flex mt-3">
              {set && <Figure label={t('gameCard.trophies')}>{set.progress}%</Figure>}
              <Figure label={t('gameCard.played')}>{formatDuration(game.playtimeSeconds)}</Figure>
            </div>
          </div>
        </div>
        {set && (
          <div className="flex items-center px-4 py-3 border-t border-line">
            <span className="font-semibold">{t('gameCard.trophies')}</span>
            {isComplete(set) && (
              <span className="ml-2 text-accent font-bold" title={t('gameCard.complete')} aria-label={t('gameCard.complete')}>
                ✓
              </span>
            )}
            <div className="ml-auto">
              <GradeCounts counts={set.counts} />
            </div>
            <span className="ml-4 text-xl text-muted" aria-hidden="true">
              ›
            </span>
          </div>
        )}
      </a>
    </li>
  );
}

function Figure({ label, children }: { label: string; children: ReactNode }) {
  return (
    <div className="mr-10">
      <div className="text-xs uppercase tracking-wide text-muted">{label}</div>
      <div className="mt-1 text-lg font-semibold tabular-nums">{children}</div>
    </div>
  );
}
