import { useTranslation } from 'react-i18next';
import { Button } from '@/components/ui/button';

export function ErrorState({ message, onRetry }: { message?: string; onRetry: () => void }) {
  const { t } = useTranslation();
  return (
    <div className="flex flex-col items-center gap-4 py-24 text-center">
      <p className="text-muted-foreground">{message ?? t('common.loadError')}</p>
      <Button onClick={onRetry}>{t('common.retry')}</Button>
    </div>
  );
}
