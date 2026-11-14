import { formatTimeAgo } from '@openpsapp/core';
import { cn } from 'cn';
import { useTranslation } from 'react-i18next';
import { Badge } from '@/components/ui/badge';
import type { SyncState } from '../_types/sync';

export function SyncBadge({ sync, now }: { sync: SyncState; now: Date }) {
  const { t, i18n } = useTranslation();
  if (sync.kind === 'none') return null;
  if (sync.kind === 'waiting') {
    return (
      <Badge variant="outline" className="px-2.5 py-1">
        {t('sync.waiting')}
      </Badge>
    );
  }
  return (
    <Badge variant="success" className="px-2.5 py-1">
      <span aria-hidden="true" className="size-1.5 rounded-full bg-success" />
      {t('sync.header', { ago: formatTimeAgo(sync.at, now, i18n.language) })}
    </Badge>
  );
}

export function SyncPill({ sync, now, className }: { sync: SyncState; now: Date; className?: string }) {
  const { t, i18n } = useTranslation();
  if (sync.kind === 'none') return null;
  const text = sync.kind === 'waiting' ? t('sync.waiting') : t('sync.short', { ago: formatTimeAgo(sync.at, now, i18n.language) });
  return (
    <span
      className={cn('flex min-w-0 items-center gap-2 rounded-full bg-muted/70 px-3 py-1.5 text-caption text-soft-foreground', className)}
    >
      <span aria-hidden="true" className={cn('size-2 shrink-0 rounded-full', sync.kind === 'synced' ? 'bg-success' : 'bg-warning')} />
      <span className="truncate">{text}</span>
    </span>
  );
}
