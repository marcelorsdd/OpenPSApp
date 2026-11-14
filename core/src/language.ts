export const LANGUAGES = [
  { code: 'pt-BR', label: 'Português' },
  { code: 'en', label: 'English' },
  { code: 'es', label: 'Español' },
] as const;

export type LanguageCode = (typeof LANGUAGES)[number]['code'];

export type LanguageChoice = 'auto' | LanguageCode;

export function parseChoice(stored: unknown): LanguageChoice {
  return LANGUAGES.some((l) => l.code === stored) ? (stored as LanguageCode) : 'auto';
}

export function resolveLanguage(tag: string | null | undefined): LanguageCode {
  const lower = (tag ?? '').toLowerCase();
  if (lower.startsWith('pt')) return 'pt-BR';
  if (lower.startsWith('es')) return 'es';
  return 'en';
}

export function effectiveLanguage(
  choice: LanguageChoice,
  consoleLanguage: string | null,
  browserLanguage: string | undefined,
): LanguageCode {
  return choice === 'auto' ? resolveLanguage(consoleLanguage ?? browserLanguage) : choice;
}
