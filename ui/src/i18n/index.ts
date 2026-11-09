import i18n from 'i18next';
import { initReactI18next } from 'react-i18next';
import en from './locales/en.json';
import es from './locales/es.json';
import ptBR from './locales/pt-BR.json';

export const LANGUAGES = [
  { code: 'pt-BR', label: 'Português' },
  { code: 'en', label: 'English' },
  { code: 'es', label: 'Español' },
] as const;

export type LanguageCode = (typeof LANGUAGES)[number]['code'];

export const resources: Record<LanguageCode, { translation: typeof ptBR }> = {
  'pt-BR': { translation: ptBR },
  en: { translation: en },
  es: { translation: es },
};

export type LanguageChoice = 'auto' | LanguageCode;

export function parseChoice(stored: unknown): LanguageChoice {
  return LANGUAGES.some((l) => l.code === stored) ? (stored as LanguageCode) : 'auto';
}

export function effectiveLanguage(choice: LanguageChoice, consoleLanguage: string | null, browserLanguage: string | undefined): LanguageCode {
  return choice === 'auto' ? resolveLanguage(consoleLanguage ?? browserLanguage) : choice;
}

export function resolveLanguage(tag: string | null | undefined): LanguageCode {
  const lower = (tag ?? '').toLowerCase();
  if (lower.startsWith('pt')) return 'pt-BR';
  if (lower.startsWith('es')) return 'es';
  return 'en';
}

const browserLanguage = typeof navigator === 'undefined' ? undefined : navigator.language;

i18n.on('languageChanged', (lng) => {
  if (typeof document !== 'undefined') document.documentElement.lang = lng;
});

i18n.use(initReactI18next).init({
  resources,
  lng: resolveLanguage(browserLanguage),
  fallbackLng: 'en',
  interpolation: { escapeValue: false },
});

export { browserLanguage };

export default i18n;
