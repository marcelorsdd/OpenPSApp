import { type LanguageChoice, LANGUAGES } from '@openpsapp/core';
import { useTranslation } from 'react-i18next';

export default function LanguagePicker({ value, onChange }: { value: LanguageChoice; onChange: (value: LanguageChoice) => void }) {
  const { t } = useTranslation();
  return (
    <select
      aria-label={t('language.label')}
      value={value}
      onChange={(e) => onChange(e.target.value as LanguageChoice)}
      className="ml-2 sm:ml-3 px-3 py-2 text-sm sm:px-4 sm:py-3 sm:text-base rounded-full bg-surface border border-line text-ink font-semibold"
    >
      <option value="auto">{t('language.auto')}</option>
      {LANGUAGES.map((l) => (
        <option key={l.code} value={l.code}>
          {l.label}
        </option>
      ))}
    </select>
  );
}
