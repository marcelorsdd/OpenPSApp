import { displayUrl, pairingLink } from '@openpsapp/core';
import { useEffect, useState } from 'react';
import { useTranslation } from 'react-i18next';
import Button from '@/components/ui/Button';
import ErrorBox from '@/components/ui/ErrorBox';
import Loading from '@/components/ui/Loading';
import Panel from '@/components/ui/Panel';
import QrCode from '@/components/ui/QrCode';
import { useApi } from '@/hooks/useApi';
import { useDateTime } from '@/hooks/useDate';
import { ApiError, cloudAction, endpoints } from '@/services/api';
import type { CloudPairing, CloudStatus } from '@/types/cloud';
import { errorMessageKey } from '@/utils/errors';

const POLL_MS = 3000;

type Action = 'link' | 'unlink' | 'sync';

export default function AccountPage() {
  const { t } = useTranslation();
  const status = useApi(endpoints.cloud);
  const [busy, setBusy] = useState(false);
  const [actionError, setActionError] = useState<string | null>(null);
  const cloud = status.data;
  const live = !!cloud?.configured && (!!cloud.pairing || cloud.linked);

  useEffect(() => {
    if (!live) return;
    const timer = window.setInterval(status.reload, POLL_MS);
    return () => window.clearInterval(timer);
  }, [live, status.reload]);

  if (!cloud) return status.error ? <ErrorBox message={status.error} onRetry={status.reload} /> : <Loading />;

  const run = (action: Action) => {
    setBusy(true);
    setActionError(null);
    void cloudAction(action)
      .catch((e: unknown) => setActionError(errorMessageKey(e instanceof ApiError ? e.code : null)))
      .then(() => {
        setBusy(false);
        status.reload();
      });
  };
  const error = actionError ? t(actionError) : cloud.lastError ? t('account.error', { message: cloud.lastError }) : null;

  return (
    <>
      <h1 className="text-3xl font-bold">{t('account.title')}</h1>
      <Panel>
        {!cloud.configured ? (
          <>
            <h2 className="text-xl font-semibold">{t('account.notConfigured')}</h2>
            <p className="mt-2 text-muted">{t('account.notConfiguredHint')}</p>
          </>
        ) : cloud.local && cloud.pairing && 'userCode' in cloud.pairing ? (
          <Pairing pairing={cloud.pairing} busy={busy} onCancel={() => run('unlink')} />
        ) : cloud.linked ? (
          <Linked cloud={cloud} busy={busy} run={cloud.local ? run : undefined} />
        ) : cloud.local ? (
          <>
            <p className="text-lg">{t('account.intro')}</p>
            <div className="mt-6">
              <Button disabled={busy} onClick={() => run('link')}>
                {busy ? t('account.connecting') : t('account.connect')}
              </Button>
            </div>
          </>
        ) : (
          <p className="text-lg">{t('account.intro')}</p>
        )}
        {cloud.configured && !cloud.local && <p className="mt-6 text-muted">{t('account.consoleOnly')}</p>}
        {error && <p className="mt-6 text-danger">{error}</p>}
      </Panel>
    </>
  );
}

function Pairing({ pairing, busy, onCancel }: { pairing: CloudPairing; busy: boolean; onCancel: () => void }) {
  const { t } = useTranslation();
  return (
    <div className="flex items-center">
      <div className="flex-1">
        <h2 className="text-xl font-semibold">{t('account.pairingTitle')}</h2>
        <div className="mt-4 font-mono text-6xl font-bold tracking-widest">{pairing.userCode}</div>
        <p className="mt-6 text-muted">{t('account.pairingHint', { url: displayUrl(pairing.verificationUri) })}</p>
        <p className="mt-2 text-muted">{t('account.expiresIn', { count: Math.max(1, Math.ceil(pairing.expiresIn / 60)) })}</p>
        <div className="mt-6">
          <Button variant="secondary" disabled={busy} onClick={onCancel}>
            {t('account.cancel')}
          </Button>
        </div>
      </div>
      <QrCode
        text={pairingLink(pairing.verificationUri, pairing.userCode)}
        label={t('account.qrLabel')}
        className="ml-8 w-56 h-56 flex-shrink-0 rounded-xl"
      />
    </div>
  );
}

function Linked({ cloud, busy, run }: { cloud: CloudStatus; busy: boolean; run?: (action: Action) => void }) {
  const { t } = useTranslation();
  const dateTime = useDateTime();
  const sync = cloud.syncing
    ? t('account.syncing')
    : cloud.lastSyncAt
      ? t('account.lastSync', { date: dateTime(cloud.lastSyncAt) })
      : t('account.neverSynced');
  return (
    <>
      <h2 className="text-xl font-semibold">{t('account.linked', { name: cloud.profileName ?? '' })}</h2>
      <p className="mt-2 text-muted">{sync}</p>
      {run && (
        <div className="mt-6 flex">
          <Button disabled={busy || cloud.syncing} onClick={() => run('sync')}>
            {t('account.syncNow')}
          </Button>
          <div className="ml-3">
            <Button variant="secondary" disabled={busy} onClick={() => run('unlink')}>
              {t('account.disconnect')}
            </Button>
          </div>
        </div>
      )}
    </>
  );
}
