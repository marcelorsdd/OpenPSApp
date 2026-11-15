import { ArrowLeftIcon, SearchIcon } from 'lucide-react';
import type { ReactNode } from 'react';
import { useTranslation } from 'react-i18next';
import { Link } from 'react-router';
import { LogoMark } from '@/components/brand/logo';
import { Button } from '@/components/ui/button';
import { UserAvatar } from './user-avatar';

export function MobileHeader({
  homeTo,
  accountTo,
  initial,
  avatarUrl,
  status,
  onSearch,
}: {
  homeTo: string;
  accountTo: string;
  initial: string;
  avatarUrl: string | null;
  status: ReactNode;
  onSearch: () => void;
}) {
  const { t } = useTranslation();
  return (
    <header className="sticky top-0 z-40 flex h-14 items-center gap-3 border-b border-border-subtle bg-background/95 px-4 backdrop-blur-sm md:hidden">
      <Link to={homeTo} aria-label={t('header.home')} className="shrink-0">
        <LogoMark />
      </Link>
      <div className="flex min-w-0 flex-1 justify-end">{status}</div>
      <Button variant="ghost" size="icon" onClick={onSearch} aria-label={t('header.searchLabel')} className="shrink-0 rounded-full">
        <SearchIcon />
      </Button>
      <Link
        to={accountTo}
        aria-label={t('nav.account')}
        className="flex size-(--control-md) shrink-0 items-center justify-center rounded-full"
      >
        <UserAvatar initial={initial} imageUrl={avatarUrl} fallbackClassName="font-bold" />
      </Link>
    </header>
  );
}

export function FocusedHeader({ backTo, title }: { backTo: string; title: string }) {
  const { t } = useTranslation();
  return (
    <header className="sticky top-0 z-40 flex h-14 items-center gap-2.5 bg-background px-2 md:hidden">
      <Button
        variant="ghost"
        size="icon-lg"
        className="rounded-full"
        aria-label={t('common.back')}
        render={<Link to={backTo} />}
        nativeButton={false}
      >
        <ArrowLeftIcon className="size-5" />
      </Button>
      <span className="font-semibold">{title}</span>
    </header>
  );
}
