import type { ImageSync } from '@openpsapp/contract/api';
import { ImageIcon } from 'lucide-react';
import { useTranslation } from 'react-i18next';
import { Card } from '@/components/ui/card';
import { Progress } from '@/components/ui/progress';

export function ImageSyncBanner({ images, className }: { images: ImageSync; className?: string }) {
  const { t } = useTranslation();
  return (
    <Card className={className} role="status">
      <div className="flex items-start gap-3 px-5 md:px-6">
        <ImageIcon className="mt-0.5 size-5 shrink-0 text-muted-foreground" />
        <div className="min-w-0 flex-1">
          <p className="font-semibold">{t('images.title')}</p>
          <p className="mt-1 text-sm text-soft-foreground">{t('images.description', { stored: images.stored, total: images.total })}</p>
          <Progress value={(images.stored * 100) / images.total} size="sm" className="mt-3" aria-label={t('images.title')} />
        </div>
      </div>
    </Card>
  );
}
