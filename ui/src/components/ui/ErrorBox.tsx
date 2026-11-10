import { useTranslation } from 'react-i18next';

export default function ErrorBox({ message, onRetry }: { message: string; onRetry: () => void }) {
  const { t } = useTranslation();
  return (
    <div className="py-24 text-center">
      <p className="text-muted">{t('common.loadError', { message })}</p>
      <button className="mt-6 px-6 py-3 rounded-xl bg-accent font-semibold" onClick={onRetry}>
        {t('common.retry')}
      </button>
    </div>
  );
}
