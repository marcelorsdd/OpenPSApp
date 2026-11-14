import { cn } from 'cn';
import { useTranslation } from 'react-i18next';
import { Outlet, useNavigate } from 'react-router';
import { paths, SYNC_DOC_URL } from '@/lib/links';
import { AccountMenu } from './_components/account-menu';
import { AppFooter } from './_components/app-footer';
import { BottomTabBar } from './_components/bottom-tab-bar';
import { CommandSearch } from './_components/command-search';
import { FocusedHeader, MobileHeader } from './_components/mobile-header';
import { SiteHeader } from './_components/site-header';
import { SyncBadge, SyncPill } from './_components/sync-badge';
import { useCommandSearch } from './_hooks/use-command-search';
import { useShell } from './_hooks/use-shell';
import { NAV_ITEMS } from './_lib/navigation';
import { matchScore } from './_lib/search';

export default function AppLayout() {
  const { t } = useTranslation();
  const navigate = useNavigate();
  const shell = useShell();
  const search = useCommandSearch();
  return (
    <div className={cn('flex min-h-dvh flex-col', !shell.focused && 'max-md:pb-tab-bar')}>
      <SiteHeader
        items={NAV_ITEMS}
        section={shell.section}
        homeTo={paths.profile}
        onSearch={() => search.setOpen(true)}
        status={<SyncBadge sync={shell.sync} now={shell.now} />}
        menu={
          <AccountMenu
            initial={shell.initial}
            name={shell.name}
            email={shell.email}
            whatIsSentUrl={SYNC_DOC_URL}
            onAccount={() => void navigate(paths.account)}
            onSignOut={shell.signOut}
          />
        }
      />
      {shell.focused ? (
        <FocusedHeader backTo={paths.account} title={t('link.phoneHeader')} />
      ) : (
        <MobileHeader
          homeTo={paths.profile}
          accountTo={paths.account}
          initial={shell.initial}
          status={<SyncPill sync={shell.sync} now={shell.now} />}
          onSearch={() => search.setOpen(true)}
        />
      )}
      <main className="flex-1">
        <Outlet />
      </main>
      <div className={cn(shell.focused && 'max-md:hidden')}>
        <AppFooter sync={shell.sync} whatIsSentUrl={SYNC_DOC_URL} />
      </div>
      {!shell.focused && <BottomTabBar items={NAV_ITEMS} section={shell.section} />}
      <CommandSearch
        open={search.open}
        onOpenChange={search.setOpen}
        query={search.query}
        onQueryChange={search.setQuery}
        groups={search.groups}
        pages={NAV_ITEMS}
        keys={search.keys}
        filter={(_, query, keywords) => matchScore(query, keywords)}
        gameHref={paths.game}
        onSelect={search.select}
      />
    </div>
  );
}
