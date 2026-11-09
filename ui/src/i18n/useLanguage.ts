import { useEffect } from 'react';
import { useTranslation } from 'react-i18next';
import { paths, useApi, type Health } from '../api';
import { formatDate } from '../format';
import { useStoredState } from '../storage';
import i18n, { browserLanguage, effectiveLanguage, parseChoice, type LanguageChoice } from '.';

export function useLanguageChoice() {
  const [stored, setChoice] = useStoredState<LanguageChoice>('ui.language', 'auto');
  const choice = parseChoice(stored);
  const health = useApi<Health>(paths.health);
  const language = effectiveLanguage(choice, health.data?.language ?? null, browserLanguage);

  useEffect(() => {
    if (i18n.language !== language) i18n.changeLanguage(language);
  }, [language]);

  return { choice, setChoice };
}

export function useDate() {
  const { i18n: instance } = useTranslation();
  return (iso: string | null) => formatDate(iso, instance.language);
}
