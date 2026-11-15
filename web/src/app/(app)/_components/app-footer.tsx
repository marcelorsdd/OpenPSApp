import { formatDateTime } from '@openpsapp/core';
import { useTranslation } from 'react-i18next';
import { SiteFooter } from '@/components/layout/site-footer';
import { SiteLinks } from '@/components/layout/site-links';
import type { SyncState } from '../_types/sync';

export function AppFooter({ sync, whatIsSentUrl }: { sync: SyncState; whatIsSentUrl: string }) {
  const { t, i18n } = useTranslation();
  const start =
    sync.kind === 'synced'
      ? t('footer.data', { name: sync.name, date: formatDateTime(sync.at, i18n.language) })
      : sync.kind === 'waiting'
        ? t('footer.dataNever', { name: sync.name })
        : null;
  return (
    <SiteFooter
      start={start}
      end={
        <>
          <a href={whatIsSentUrl} target="_blank" rel="noreferrer" className="font-semibold text-link hover:text-link-hover">
            {t('footer.whatIsSent')}
          </a>
          <SiteLinks />
        </>
      }
    />
  );
}
