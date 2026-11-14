import type { DeviceInfo } from '@openpsapp/contract/pairing';
import { formatDate, formatDateTime } from '@openpsapp/core';
import { useTranslation } from 'react-i18next';
import { Link } from 'react-router';
import { ConsoleTile } from '@/components/console-tile';
import { Badge } from '@/components/ui/badge';
import { Button } from '@/components/ui/button';
import { Card } from '@/components/ui/card';
import { Separator } from '@/components/ui/separator';
import type { AccountTotals } from '../_types/account';

export function ConsoleCard({
  device,
  totals,
  changeTo,
  onDisconnect,
}: {
  device: DeviceInfo;
  totals: AccountTotals | null;
  changeTo: string;
  onDisconnect: () => void;
}) {
  const { t, i18n } = useTranslation();
  const facts = [
    { label: t('account.console.connectedOn'), value: formatDate(device.createdAt, i18n.language) },
    { label: t('account.console.syncs'), value: t('account.console.syncInterval') },
    totals && {
      label: t('account.console.onAccount'),
      value: t('account.console.onAccountValue', {
        games: t('profile.games', { count: totals.games }),
        trophies: t('account.console.trophies', { count: totals.trophies }),
      }),
    },
  ].filter((fact) => fact !== null);
  return (
    <Card className="px-5 max-md:gap-4 md:px-6">
      <div className="flex items-center justify-between gap-3">
        <div>
          <h2 className="text-body font-semibold">{t('account.console.title')}</h2>
          <p className="mt-0.5 text-sm text-muted-foreground">
            <span className="md:hidden">{t('account.console.hintLong')}</span>
            <span className="max-md:hidden">{t('account.console.hint')}</span>
          </p>
        </div>
        {device.lastSyncAt ? (
          <Badge variant="success" className="shrink-0">
            <span aria-hidden="true" className="size-1.5 rounded-full bg-success" />
            {t('account.console.synced')}
          </Badge>
        ) : (
          <Badge variant="outline" className="shrink-0">
            {t('account.console.waiting')}
          </Badge>
        )}
      </div>
      <div className="flex flex-wrap items-center gap-4 md:mt-1">
        <ConsoleTile />
        <div className="min-w-0 grow basis-50">
          <div className="text-lead font-semibold">{t('account.console.profile', { name: device.profileName })}</div>
          <div className="text-sm text-muted-foreground">
            {device.lastSyncAt
              ? t('account.console.lastSync', { date: formatDateTime(device.lastSyncAt, i18n.language) })
              : t('account.console.neverSynced')}
          </div>
        </div>
        <div className="grid gap-2 max-md:w-full max-md:grid-cols-2 md:flex">
          <Button
            variant="outline"
            className="max-md:h-(--control-xl) max-md:rounded-xl max-md:font-semibold"
            render={<Link to={changeTo} />}
            nativeButton={false}
          >
            {t('account.console.change')}
          </Button>
          <Button
            variant="destructive-outline"
            className="max-md:h-(--control-xl) max-md:rounded-xl max-md:font-semibold"
            onClick={onDisconnect}
          >
            {t('account.console.disconnect')}
          </Button>
        </div>
      </div>
      <Separator className="max-md:hidden" />
      <dl className="grid grid-cols-3 gap-4 max-md:hidden">
        {facts.map((fact) => (
          <div key={fact.label}>
            <dt className="text-xs text-muted-foreground">{fact.label}</dt>
            <dd className="mt-0.5 font-medium">{fact.value}</dd>
          </div>
        ))}
      </dl>
    </Card>
  );
}
