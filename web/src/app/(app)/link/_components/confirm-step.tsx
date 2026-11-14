import { CheckIcon, InfoIcon, QrCodeIcon, TriangleAlertIcon } from 'lucide-react';
import { useTranslation } from 'react-i18next';
import { ConsoleTile } from '@/components/console-tile';
import { Alert, AlertDescription, AlertTitle } from '@/components/ui/alert';
import { Button } from '@/components/ui/button';
import { Spinner } from '@/components/ui/spinner';

export function ConfirmStep({
  profileName,
  code,
  replaces,
  whatIsSentUrl,
  canScan,
  pending,
  onConfirm,
  onOtherCode,
  onScanAgain,
}: {
  profileName: string;
  code: string;
  replaces: string | null;
  whatIsSentUrl: string;
  canScan: boolean;
  pending: boolean;
  onConfirm: () => void;
  onOtherCode: () => void;
  onScanAgain: () => void;
}) {
  const { t } = useTranslation();
  const sends = [
    t('link.confirm.sendsProfile'),
    t('link.confirm.sendsGames'),
    t('link.confirm.sendsTrophies'),
    t('link.confirm.sendsImages'),
  ];
  return (
    <section className="basis-form-wide pb-40 md:rounded-2xl md:border md:border-border md:bg-card md:p-8">
      <div className="flex items-center gap-3 md:gap-3.5">
        <ConsoleTile size="sm" className="max-md:size-13" />
        <div className="min-w-0">
          <h1 className="text-title font-extrabold md:text-xl md:font-bold">{t('link.confirm.title', { name: profileName })}</h1>
          <div className="text-muted-foreground max-md:hidden">
            {t('link.confirm.code')} <span className="font-mono font-semibold text-foreground">{code}</span>
          </div>
        </div>
      </div>
      <div className="mt-4.5 rounded-xl border border-border bg-card p-3.5 text-center md:hidden">
        <div className="text-xs text-muted-foreground">{t('link.confirm.code')}</div>
        <div className="mt-0.5 font-mono text-3xl font-bold tracking-widest">{code}</div>
      </div>
      <Alert variant="warning" className="mt-3 md:mt-5">
        <TriangleAlertIcon />
        <AlertTitle>{t('link.confirm.checkTitle')}</AlertTitle>
        <AlertDescription>
          <span className="md:hidden">{t('link.confirm.checkShort')}</span>
          <span className="max-md:hidden">{t('link.confirm.check', { code })}</span>
        </AlertDescription>
      </Alert>
      {replaces && (
        <Alert className="mt-2.5">
          <InfoIcon />
          <AlertTitle className="max-md:hidden">{t('link.confirm.replacesTitle')}</AlertTitle>
          <AlertDescription>
            <span className="md:hidden">{t('link.confirm.replacesShort')}</span>
            <span className="max-md:hidden">{t('link.confirm.replaces', { name: replaces })}</span>
          </AlertDescription>
        </Alert>
      )}
      <p className="mt-4 text-sm text-muted-foreground md:hidden">{t('link.confirm.sendsShort')}</p>
      <div className="max-md:hidden">
        <div className="mt-5 text-caption font-medium">{t('link.confirm.sendsTitle')}</div>
        <ul className="mt-2 grid grid-cols-2 gap-2 text-sm text-soft-foreground">
          {sends.map((item) => (
            <li key={item} className="flex items-center gap-2">
              <CheckIcon className="size-3.5 text-success" />
              {item}
            </li>
          ))}
        </ul>
        <a
          href={whatIsSentUrl}
          target="_blank"
          rel="noreferrer"
          className="mt-2.5 inline-block text-caption font-medium text-link hover:text-link-hover"
        >
          {t('link.confirm.never')}
        </a>
      </div>
      <div className="mt-6 flex justify-end gap-2 max-md:hidden">
        <Button variant="outline" size="lg" className="font-medium" onClick={onOtherCode} disabled={pending}>
          {t('link.confirm.otherCode')}
        </Button>
        {canScan && (
          <Button variant="secondary" size="lg" className="font-medium" onClick={onScanAgain} disabled={pending}>
            <QrCodeIcon />
            {t('link.confirm.scanAgain')}
          </Button>
        )}
        <Button size="lg" onClick={onConfirm} disabled={pending}>
          {pending && <Spinner />}
          {t('link.confirm.submit')}
        </Button>
      </div>
      <div className="fixed inset-x-4 bottom-9 z-30 flex flex-col gap-2.5 md:hidden">
        <Button size="xl" className="h-12.5 font-bold" onClick={onConfirm} disabled={pending}>
          {pending && <Spinner />}
          {t('link.confirm.submit')}
        </Button>
        <div className={canScan ? 'grid grid-cols-2 gap-2.5' : 'grid'}>
          {canScan && (
            <Button variant="outline" size="xl" className="h-12.5" onClick={onScanAgain} disabled={pending}>
              <QrCodeIcon />
              {t('link.confirm.scanAgainShort')}
            </Button>
          )}
          <Button variant="outline" size="xl" className="h-12.5" onClick={onOtherCode} disabled={pending}>
            {t('link.confirm.typeCode')}
          </Button>
        </div>
      </div>
    </section>
  );
}
