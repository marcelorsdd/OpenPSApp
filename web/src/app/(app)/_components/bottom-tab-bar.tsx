import { cn } from 'cn';
import { TrophyIcon, UserRoundIcon } from 'lucide-react';
import { useTranslation } from 'react-i18next';
import { NavLink } from 'react-router';
import { ConsoleGlyph } from '@/components/brand/console-glyph';
import type { NavItem, Section } from '../_types/navigation';

const ICONS: Record<Section, React.ReactNode> = {
  profile: <UserRoundIcon className="size-6" />,
  trophies: <TrophyIcon className="size-6" />,
  account: <ConsoleGlyph className="size-6" />,
};

export function BottomTabBar({ items, section }: { items: NavItem[]; section: Section | null }) {
  const { t } = useTranslation();
  return (
    <nav
      aria-label={t('nav.label')}
      className="pb-safe fixed inset-x-0 bottom-0 z-40 grid grid-cols-3 border-t border-accent bg-nav/95 px-2 pt-2 backdrop-blur-sm md:hidden"
    >
      {items.map((item) => (
        <NavLink
          key={item.section}
          to={item.to}
          aria-current={item.section === section ? 'page' : undefined}
          className={cn(
            'flex flex-col items-center gap-0.5 py-1.5 text-xs font-semibold',
            item.section === section ? 'text-foreground' : 'text-muted-foreground',
          )}
        >
          {ICONS[item.section]}
          {t(`nav.${item.section}`)}
        </NavLink>
      ))}
    </nav>
  );
}
