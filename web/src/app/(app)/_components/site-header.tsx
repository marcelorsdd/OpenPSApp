import { cn } from 'cn';
import { SearchIcon } from 'lucide-react';
import type { ReactNode } from 'react';
import { useTranslation } from 'react-i18next';
import { Link, NavLink } from 'react-router';
import { Logo } from '@/components/brand/logo';
import { Kbd } from '@/components/ui/kbd';
import type { NavItem, Section } from '../_types/navigation';

export function SiteHeader({
  items,
  section,
  homeTo,
  onSearch,
  status,
  menu,
}: {
  items: NavItem[];
  section: Section | null;
  homeTo: string;
  onSearch: () => void;
  status: ReactNode;
  menu: ReactNode;
}) {
  const { t } = useTranslation();
  return (
    <header className="sticky top-0 z-40 hidden items-center justify-between gap-4 border-b border-border-subtle bg-background/95 px-6 py-3.5 backdrop-blur-sm md:flex lg:px-12">
      <div className="flex items-center gap-7">
        <Link to={homeTo} aria-label={t('header.home')}>
          <Logo />
        </Link>
        <nav aria-label={t('nav.label')} className="flex gap-1">
          {items.map((item) => (
            <NavLink
              key={item.section}
              to={item.to}
              aria-current={item.section === section ? 'page' : undefined}
              className={cn(
                'rounded-md px-3 py-2 text-sm font-medium transition-colors',
                item.section === section ? 'bg-muted text-foreground' : 'text-muted-foreground hover:text-foreground',
              )}
            >
              {t(`nav.${item.section}`)}
            </NavLink>
          ))}
        </nav>
      </div>
      <div className="flex items-center gap-3">
        <button
          type="button"
          onClick={onSearch}
          aria-label={t('header.searchLabel')}
          className="flex h-(--control-md) w-search-trigger items-center gap-2 rounded-lg border border-input px-2.5 text-sm text-muted-foreground transition-colors outline-none hover:bg-accent/50 focus-visible:border-ring focus-visible:ring-3 focus-visible:ring-ring/30 max-lg:w-auto"
        >
          <SearchIcon className="size-4" />
          <span className="flex-1 truncate text-left max-lg:sr-only">{t('header.search')}</span>
          <Kbd className="max-lg:hidden">⌘K</Kbd>
        </button>
        <span className="max-lg:hidden">{status}</span>
        {menu}
      </div>
    </header>
  );
}
