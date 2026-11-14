import { formatDuration } from '@openpsapp/core';
import { TrophyIcon, UserRoundIcon } from 'lucide-react';
import { useTranslation } from 'react-i18next';
import { ConsoleGlyph } from '@/components/brand/console-glyph';
import { GameCover } from '@/components/game-cover';
import { GradeBadge } from '@/components/grade-badge';
import { Button } from '@/components/ui/button';
import {
  Command,
  CommandDialog,
  CommandEmpty,
  CommandGroup,
  CommandInput,
  CommandItem,
  CommandList,
  CommandSeparator,
  CommandShortcut,
} from '@/components/ui/command';
import { Kbd } from '@/components/ui/kbd';
import type { NavItem, Section, ShortcutKeys } from '../_types/navigation';
import type { SearchGroups } from '../_types/search';

const PAGE_ICONS: Record<Section, React.ReactNode> = {
  profile: <UserRoundIcon className="text-muted-foreground" />,
  trophies: <TrophyIcon className="text-muted-foreground" />,
  account: <ConsoleGlyph className="size-4 text-muted-foreground" />,
};

export function CommandSearch({
  open,
  onOpenChange,
  query,
  onQueryChange,
  groups,
  pages,
  keys,
  filter,
  gameHref,
  onSelect,
}: {
  open: boolean;
  onOpenChange: (open: boolean) => void;
  query: string;
  onQueryChange: (query: string) => void;
  groups: SearchGroups;
  pages: NavItem[];
  keys: ShortcutKeys;
  filter: (value: string, search: string, keywords?: string[]) => number;
  gameHref: (titleId: string) => string;
  onSelect: (to: string) => void;
}) {
  const { t } = useTranslation();
  const pageLabel = (section: Section) => (section === 'account' ? t('nav.accountAndConsole') : t(`nav.${section}`));
  return (
    <CommandDialog open={open} onOpenChange={onOpenChange} title={t('search.title')} description={t('search.description')}>
      <Command filter={filter} loop>
        <div className="flex items-center gap-2 border-b border-border pr-3">
          <CommandInput value={query} onValueChange={onQueryChange} placeholder={t('search.placeholder')} autoFocus />
          <Kbd className="max-md:hidden">Esc</Kbd>
          <Button variant="ghost" size="sm" className="md:hidden" onClick={() => onOpenChange(false)}>
            {t('common.cancel')}
          </Button>
        </div>
        <CommandList>
          <CommandEmpty>{t('search.empty')}</CommandEmpty>
          {groups.games.length > 0 && (
            <CommandGroup heading={t('search.games')}>
              {groups.games.map((game) => (
                <CommandItem key={game.value} value={game.value} keywords={[game.name]} onSelect={() => onSelect(gameHref(game.titleId))}>
                  <GameCover src={game.iconUrl} className="size-cover-xs rounded-sm" />
                  <span className="min-w-0 flex-1 truncate font-medium">{game.name}</span>
                  <span className="shrink-0 text-caption text-muted-foreground">
                    {game.progress === null
                      ? formatDuration(game.playtimeSeconds)
                      : t('search.gameMeta', { progress: game.progress, duration: formatDuration(game.playtimeSeconds) })}
                  </span>
                </CommandItem>
              ))}
            </CommandGroup>
          )}
          {groups.trophies.length > 0 && (
            <>
              <CommandSeparator />
              <CommandGroup heading={t('search.trophies')}>
                {groups.trophies.map((trophy) => (
                  <CommandItem
                    key={trophy.value}
                    value={trophy.value}
                    keywords={[trophy.name, trophy.game]}
                    disabled={!trophy.titleId}
                    onSelect={() => trophy.titleId && onSelect(gameHref(trophy.titleId))}
                  >
                    <GameCover src={trophy.iconUrl} className="size-cover-xs rounded-sm" />
                    <span className="min-w-0 flex-1 truncate">
                      <span className="font-medium">{trophy.name}</span>{' '}
                      <span className="text-muted-foreground">{t('search.trophyGame', { game: trophy.game })}</span>
                    </span>
                    <GradeBadge grade={trophy.grade} />
                  </CommandItem>
                ))}
              </CommandGroup>
            </>
          )}
          <CommandSeparator />
          <CommandGroup heading={t('search.pages')}>
            {pages.map((page) => (
              <CommandItem
                key={page.section}
                value={`page:${page.section}`}
                keywords={[pageLabel(page.section)]}
                onSelect={() => onSelect(page.to)}
              >
                {PAGE_ICONS[page.section]}
                <span className="flex-1">{pageLabel(page.section)}</span>
                <CommandShortcut className="max-md:hidden">G {keys[page.section].toUpperCase()}</CommandShortcut>
              </CommandItem>
            ))}
          </CommandGroup>
        </CommandList>
        <div className="flex items-center justify-between gap-4 border-t border-border px-4 py-2.5 text-xs text-muted-foreground max-md:hidden">
          <span>
            {t('search.footer', {
              games: t('search.footerGames', { count: groups.totals.games }),
              trophies: t('search.footerTrophies', { count: groups.totals.trophies }),
            })}
          </span>
          <span>{t('search.keys')}</span>
        </div>
      </Command>
    </CommandDialog>
  );
}
