import { useTranslation } from 'react-i18next';

export default function Loading() {
  const { t } = useTranslation();
  return <div className="py-24 text-center text-muted">{t('common.loading')}</div>;
}
