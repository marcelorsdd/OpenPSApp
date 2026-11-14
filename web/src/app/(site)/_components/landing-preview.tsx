import { useTranslation } from 'react-i18next';
import { LogoMark } from '@/components/brand/logo';
import { GameCover } from '@/components/game-cover';
import { TrophyGlyph } from '@/components/trophy-glyph';
import { Progress } from '@/components/ui/progress';

export function LandingPreview() {
  const { t } = useTranslation();
  return (
    <div aria-hidden="true" className="relative h-115 min-w-0 grow basis-105 max-md:hidden">
      <div className="absolute top-0 right-0 bottom-15 left-10 overflow-hidden rounded-4xl border border-border bg-brand-glow">
        <LogoMark className="absolute top-1/2 left-1/2 size-48 -translate-1/2 bg-transparent opacity-40" />
      </div>
      <div className="absolute bottom-0 left-0 w-80 rounded-3xl border border-border bg-card p-5 shadow-dialog">
        <div className="flex items-center gap-3">
          <GameCover src={null} className="size-13 rounded-xl" />
          <div>
            <div className="font-bold">{t('landing.preview.game')}</div>
            <div className="text-sm text-muted-foreground">{t('landing.preview.gameMeta')}</div>
          </div>
        </div>
        <Progress value={50} className="mt-3.5" />
      </div>
      <div className="absolute top-6 right-4 flex items-center gap-2.5 rounded-xl border border-border bg-card/90 px-3.5 py-2.5">
        <span className="flex size-cover-sm items-center justify-center rounded-md bg-muted">
          <TrophyGlyph grade="silver" className="size-5" />
        </span>
        <div className="text-sm">
          <div className="font-bold">{t('landing.preview.trophy')}</div>
          <div className="text-muted-foreground">{t('landing.preview.trophyMeta')}</div>
        </div>
      </div>
    </div>
  );
}
