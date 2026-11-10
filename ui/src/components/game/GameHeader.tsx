import { useState, type ReactNode } from 'react';
import { useTranslation } from 'react-i18next';
import GradeCounts from '@/components/trophies/GradeCounts';
import Cover from '@/components/ui/Cover';
import ProgressBar from '@/components/ui/ProgressBar';
import { useDate } from '@/hooks/useDate';
import type { Game, TrophySet } from '@/types/api';
import { formatDuration } from '@/utils/format';

export default function GameHeader({ game, set, actions }: { game?: Game | null; set?: TrophySet | null; actions?: ReactNode }) {
  const { t } = useTranslation();
  const date = useDate();
  const [failedHero, setFailedHero] = useState<string | null>(null);
  const hero = game?.heroUrl && game.heroUrl !== failedHero ? game.heroUrl : null;
  return (
    <>
      {hero && <HeroArt key={hero} src={hero} onError={() => setFailedHero(hero)} />}
      <div className={`relative flex items-start ${hero ? '-mt-24 px-6' : ''}`}>
        <Cover src={game?.iconUrl ?? set?.iconUrl ?? null} size="w-40 h-40" />
        <div className="ml-6 flex-1 min-w-0">
          <h1 className="text-3xl font-bold drop-shadow-lg">{game?.name ?? set?.name}</h1>
          {game && (
            <p className="text-muted mt-2">
              {t('game.summary', { duration: formatDuration(game.playtimeSeconds), date: date(game.lastPlayedAt) })}
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
    </>
  );
}

function HeroArt({ src, onError }: { src: string; onError: () => void }) {
  const [loaded, setLoaded] = useState(false);
  return (
    <div className="relative h-80 rounded-3xl overflow-hidden bg-surface" aria-hidden="true">
      <img
        src={src}
        alt=""
        decoding="async"
        onLoad={() => setLoaded(true)}
        onError={onError}
        className={`w-full h-full object-cover transition-opacity duration-700 ${loaded ? 'opacity-100' : 'opacity-0'}`}
      />
      <div className="absolute top-0 left-0 w-full h-full bg-gradient-to-t from-bg via-bg/60 to-transparent" />
    </div>
  );
}
