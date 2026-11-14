import { cn } from 'cn';
import { Link } from 'react-router';
import { GameCover } from '@/components/game-cover';
import { Progress } from '@/components/ui/progress';
import type { ProfileGame } from '../_types/profile';

export function GameCard({
  game,
  href,
  meta,
  metaShort,
  progressLabel,
  noTrophiesLabel,
}: {
  game: ProfileGame;
  href: string;
  meta: string;
  metaShort: string;
  progressLabel: string | null;
  noTrophiesLabel: string;
}) {
  return (
    <Link
      to={href}
      className="block w-scroller shrink-0 snap-start overflow-hidden rounded-2xl border border-border bg-card transition-colors hover:bg-accent/40 md:w-auto md:rounded-3xl"
    >
      <div className="relative h-27.5 bg-muted md:aspect-video md:h-auto">
        {game.heroUrl ? (
          <img src={game.heroUrl} alt="" loading="lazy" className="size-full object-cover" />
        ) : (
          <div className="flex size-full items-center justify-center bg-brand-tile">
            <GameCover src={game.iconUrl} className="size-cover-lg rounded-lg md:size-cover-xl" />
          </div>
        )}
        <GameCover
          src={game.iconUrl}
          className={cn('absolute -bottom-5.5 left-3.5 size-cover-lg rounded-lg shadow-ring-card max-md:hidden')}
        />
      </div>
      <div className="px-3 pt-2.5 pb-3 md:px-4 md:pt-7.5 md:pb-4">
        <div className="truncate font-bold md:text-lead">{game.name}</div>
        <div className="text-caption text-muted-foreground md:mt-1 md:text-sm">
          <span className="md:hidden">{metaShort}</span>
          <span className="max-md:hidden">{meta}</span>
        </div>
        {game.set ? (
          <>
            <div className="mt-2 flex items-center gap-2.5 md:mt-3.5">
              <Progress value={game.set.progress} size="sm" className="flex-1" />
              <span className="text-sm font-bold tabular-nums max-md:hidden">{game.set.progress}%</span>
            </div>
            <div className="mt-1.5 text-caption text-muted-foreground max-md:hidden">{progressLabel}</div>
          </>
        ) : (
          <div className="mt-3.5 text-caption text-muted-foreground max-md:hidden">{noTrophiesLabel}</div>
        )}
      </div>
    </Link>
  );
}
