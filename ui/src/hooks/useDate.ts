import { formatDate, formatDateTime } from '@openpsapp/core';
import { useTranslation } from 'react-i18next';

export function useDate() {
  const { i18n: instance } = useTranslation();
  return (iso: string | null) => formatDate(iso, instance.language);
}

export function useDateTime() {
  const { i18n: instance } = useTranslation();
  return (iso: string | null) => formatDateTime(iso, instance.language);
}
