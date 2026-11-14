import { effectiveLanguage, type LanguageChoice, type LanguageCode, parseChoice } from '@openpsapp/core';
import i18n from 'i18next';
import { initReactI18next } from 'react-i18next';
import { readStored } from '@/lib/storage';
import en from './locales/en.json';
import es from './locales/es.json';
import ptBR from './locales/pt-BR.json';

export const resources: Record<LanguageCode, { translation: typeof en }> = {
  'pt-BR': { translation: ptBR },
  en: { translation: en },
  es: { translation: es },
};

export const LANGUAGE_STORAGE_KEY = 'openpsapp.language';

export function readLanguageChoice(): LanguageChoice {
  return parseChoice(readStored<unknown>(LANGUAGE_STORAGE_KEY, null));
}

export function languageFor(choice: LanguageChoice): LanguageCode {
  return effectiveLanguage(choice, null, typeof navigator === 'undefined' ? undefined : navigator.language);
}

i18n.on('languageChanged', (language) => {
  if (typeof document !== 'undefined') document.documentElement.lang = language;
});

void i18n.use(initReactI18next).init({
  resources,
  lng: languageFor(readLanguageChoice()),
  fallbackLng: 'en',
  interpolation: { escapeValue: false },
});

export default i18n;
