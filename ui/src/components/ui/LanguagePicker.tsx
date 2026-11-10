import { useTranslation } from 'react-i18next';
import { LANGUAGES, type LanguageChoice } from '@/i18n';

export default function LanguagePicker({ value, onChange }: { value: LanguageChoice; onChange: (value: LanguageChoice) => void }) {
  const { t } = useTranslation();
  return (
    <select
      aria-label={t('language.label')}
      value={value}
      onChange={(e) => onChange(e.target.value as LanguageChoice)}
      className="ml-3 px-4 py-3 rounded-full bg-surface border border-line text-ink font-semibold"
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
