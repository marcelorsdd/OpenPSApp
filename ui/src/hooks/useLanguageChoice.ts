import { effectiveLanguage, type LanguageChoice, parseChoice } from '@openpsapp/core';
import { useEffect } from 'react';
import { useStoredState } from '@/hooks/useStoredState';
import i18n, { browserLanguage } from '@/i18n';

export interface LanguageState {
  choice: LanguageChoice;
  setChoice: (choice: LanguageChoice) => void;
}

export function useLanguageChoice(consoleLanguage: string | null): LanguageState {
  const [stored, setChoice] = useStoredState<LanguageChoice>('ui.language', 'auto');
  const choice = parseChoice(stored);
  const language = effectiveLanguage(choice, consoleLanguage, browserLanguage);

  useEffect(() => {
    if (i18n.language !== language) void i18n.changeLanguage(language);
  }, [language]);

  return { choice, setChoice };
}
