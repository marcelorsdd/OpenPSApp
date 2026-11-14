import { type LanguageCode, resolveLanguage } from '@openpsapp/core';
import i18n from 'i18next';
import { initReactI18next } from 'react-i18next';
import en from './locales/en.json';
import es from './locales/es.json';
import ptBR from './locales/pt-BR.json';

export const resources: Record<LanguageCode, { translation: typeof ptBR }> = {
  'pt-BR': { translation: ptBR },
  en: { translation: en },
  es: { translation: es },
};

export const browserLanguage = typeof navigator === 'undefined' ? undefined : navigator.language;

i18n.on('languageChanged', (lng) => {
  if (typeof document !== 'undefined') document.documentElement.lang = lng;
});

void i18n.use(initReactI18next).init({
  resources,
  lng: resolveLanguage(browserLanguage),
  fallbackLng: 'en',
  interpolation: { escapeValue: false },
});

export default i18n;
