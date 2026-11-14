import { type LanguageChoice, parseChoice } from '@openpsapp/core';
import { useTranslation } from 'react-i18next';
import { useStoredState } from '@/hooks/use-stored-state';
import { LANGUAGE_STORAGE_KEY, languageFor } from '@/i18n';

export function useLanguageChoice() {
  const { i18n } = useTranslation();
  const [stored, setStored] = useStoredState<LanguageChoice>(LANGUAGE_STORAGE_KEY, 'auto');
  return {
    choice: parseChoice(stored),
    setChoice: (choice: LanguageChoice) => {
      setStored(choice);
      void i18n.changeLanguage(languageFor(choice));
    },
  };
}
