import type { ImageSync } from '@openpsapp/contract/api';
import { formatTimeAgo } from '@openpsapp/core';
import { cn } from 'cn';
import { useTranslation } from 'react-i18next';
import { Badge } from '@/components/ui/badge';
import type { SyncState } from '../_types/sync';

export function SyncBadge({ sync, images, now }: { sync: SyncState; images: ImageSync | null; now: Date }) {
  const { t, i18n } = useTranslation();
  if (sync.kind === 'none') return null;
  if (sync.kind === 'synced' && images) {
    return (
      <Badge variant="outline" className="px-2.5 py-1">
        <span aria-hidden="true" className="size-1.5 animate-pulse rounded-full bg-warning" />
        {t('images.badge', { stored: images.stored, total: images.total })}
      </Badge>
    );
  }
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

export function SyncPill({ sync, images, now, className }: { sync: SyncState; images: ImageSync | null; now: Date; className?: string }) {
  const { t, i18n } = useTranslation();
  if (sync.kind === 'none') return null;
  const sending = sync.kind === 'synced' && images;
  const text =
    sync.kind === 'waiting'
      ? t('sync.waiting')
      : sending
        ? t('images.badge', { stored: images.stored, total: images.total })
        : t('sync.short', { ago: formatTimeAgo(sync.at, now, i18n.language) });
  return (
    <span
      className={cn('flex min-w-0 items-center gap-2 rounded-full bg-muted/70 px-3 py-1.5 text-caption text-soft-foreground', className)}
    >
      <span
        aria-hidden="true"
        className={cn(
          'size-2 shrink-0 rounded-full',
          sync.kind === 'synced' && !sending ? 'bg-success' : 'bg-warning',
          sending && 'animate-pulse',
        )}
      />
      <span className="truncate">{text}</span>
    </span>
  );
}
