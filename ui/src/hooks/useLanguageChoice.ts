import { useEffect } from 'react';
import { useApi } from '@/hooks/useApi';
import { useStoredState } from '@/hooks/useStoredState';
import i18n, { browserLanguage, effectiveLanguage, parseChoice, type LanguageChoice } from '@/i18n';
import { endpoints } from '@/services/api';

export function useLanguageChoice() {
  const [stored, setChoice] = useStoredState<LanguageChoice>('ui.language', 'auto');
  const choice = parseChoice(stored);
  const health = useApi(endpoints.health);
  const language = effectiveLanguage(choice, health.data?.language ?? null, browserLanguage);

  useEffect(() => {
    if (i18n.language !== language) i18n.changeLanguage(language);
  }, [language]);

  return { choice, setChoice };
}
