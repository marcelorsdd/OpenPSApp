import { cn } from 'cn';
import { CameraOffIcon, XIcon } from 'lucide-react';
import type { RefObject } from 'react';
import { useTranslation } from 'react-i18next';
import { Button } from '@/components/ui/button';
import { Spinner } from '@/components/ui/spinner';
import type { CameraStatus } from '../_types/link';

const CORNERS = [
  'top-0 left-0 border-t-4 border-l-4 rounded-tl-3xl',
  'top-0 right-0 border-t-4 border-r-4 rounded-tr-3xl',
  'bottom-0 left-0 border-b-4 border-l-4 rounded-bl-3xl',
  'bottom-0 right-0 border-b-4 border-r-4 rounded-br-3xl',
];

export function ScannerStep({
  videoRef,
  status,
  rejected,
  onClose,
  onTypeCode,
  onRetry,
}: {
  videoRef: RefObject<HTMLVideoElement | null>;
  status: CameraStatus;
  rejected: boolean;
  onClose: () => void;
  onTypeCode: () => void;
  onRetry: () => void;
}) {
  const { t } = useTranslation();
  const problem =
    status === 'denied'
      ? { title: t('link.scanner.deniedTitle'), text: t('link.scanner.denied'), retry: true }
      : status === 'unavailable'
        ? { title: t('link.scanner.unavailableTitle'), text: t('link.scanner.unavailable'), retry: false }
        : status === 'failed'
          ? { title: t('link.scanner.failedTitle'), text: t('link.scanner.failed'), retry: true }
          : null;
  return (
    <div
      role="dialog"
      aria-modal="true"
      aria-label={t('link.scanner.title')}
      className="fixed inset-0 z-50 overflow-hidden bg-black text-white"
    >
      <video ref={videoRef} muted playsInline className={cn('absolute inset-0 size-full object-cover', problem && 'hidden')} />
      <div aria-hidden="true" className="absolute inset-x-0 top-0 h-28 bg-linear-to-b from-black/70 to-transparent" />
      <div aria-hidden="true" className="absolute inset-x-0 bottom-0 h-1/2 bg-linear-to-t from-black/85 via-black/60 to-transparent" />
      {!problem && (
        <div aria-hidden="true" className="absolute top-viewfinder left-1/2 size-67.5 -translate-1/2 rounded-3xl shadow-viewfinder">
          {CORNERS.map((corner) => (
            <span key={corner} className={cn('absolute size-11 border-primary', corner)} />
          ))}
          {status === 'scanning' && <span className="absolute inset-x-6 h-0.5 animate-scan rounded-full bg-primary/70" />}
        </div>
      )}
      <div className="absolute inset-x-0 top-0 flex items-center justify-between p-4">
        <Button
          variant="ghost"
          size="icon-xl"
          className="rounded-full bg-white/12 text-white hover:bg-white/20"
          onClick={onClose}
          aria-label={t('link.scanner.close')}
        >
          <XIcon className="size-5" />
        </Button>
        <span className="font-semibold">{t('link.scanner.title')}</span>
        <span className="size-11" />
      </div>
      {problem ? (
        <div className="absolute inset-x-6 top-1/3 flex flex-col items-center text-center">
          <span className="flex size-14 items-center justify-center rounded-full bg-white/10">
            <CameraOffIcon className="size-6" />
          </span>
          <div className="mt-4 text-lead font-semibold">{problem.title}</div>
          <p className="mt-1 max-w-sm text-sm text-white/70">{problem.text}</p>
          {problem.retry && (
            <Button variant="ghost" className="mt-4 rounded-full bg-white/10 text-white hover:bg-white/20" onClick={onRetry}>
              {t('common.retry')}
            </Button>
          )}
        </div>
      ) : (
        <div className="absolute inset-x-6 top-viewfinder-hint text-center">
          <div className="text-lead font-semibold">{t('link.scanner.hint')}</div>
          <div className="mt-1 text-sm text-white/70">{t('link.scanner.where')}</div>
          <div
            role="status"
            className={cn(
              'mx-auto mt-4 flex w-max max-w-full items-center gap-2 rounded-full px-3 py-1.5 text-caption',
              rejected ? 'bg-destructive/25 text-destructive-soft-foreground' : 'bg-white/10',
            )}
          >
            {!rejected && <Spinner className="size-3.5" />}
            {rejected ? t('link.scanner.notOurs') : t('link.scanner.searching')}
          </div>
        </div>
      )}
      <div className="absolute inset-x-4 bottom-9">
        <Button variant="ghost" size="xl" className="h-12.5 w-full bg-white/12 text-white hover:bg-white/20" onClick={onTypeCode}>
          {t('link.scanner.type')}
        </Button>
        <p className="mt-3 text-center text-xs text-white/60">{t('link.scanner.privacy')}</p>
      </div>
    </div>
  );
}
