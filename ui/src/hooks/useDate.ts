import { useTranslation } from 'react-i18next';
import { formatDate } from '@/utils/format';

export function useDate() {
  const { i18n: instance } = useTranslation();
  return (iso: string | null) => formatDate(iso, instance.language);
}
