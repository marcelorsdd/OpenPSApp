import { useTranslation } from 'react-i18next';
import Button from '@/components/ui/Button';

export default function ErrorBox({ message, onRetry }: { message: string; onRetry: () => void }) {
  const { t } = useTranslation();
  return (
    <div className="py-24 text-center">
      <p className="text-muted">{t('common.loadError', { message })}</p>
      <div className="mt-6">
        <Button onClick={onRetry}>{t('common.retry')}</Button>
      </div>
    </div>
  );
}
