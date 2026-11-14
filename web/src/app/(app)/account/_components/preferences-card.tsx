import type { LanguageChoice } from '@openpsapp/core';
import { useTranslation } from 'react-i18next';
import { Card } from '@/components/ui/card';
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from '@/components/ui/select';
import type { LanguageOption } from '../_types/account';

export function PreferencesCard({
  options,
  language,
  onLanguageChange,
}: {
  options: LanguageOption[];
  language: LanguageChoice;
  onLanguageChange: (language: LanguageChoice) => void;
}) {
  const { t } = useTranslation();
  return (
    <Card className="px-5 md:px-6">
      <h2 className="text-body font-semibold">{t('account.preferences.title')}</h2>
      <div className="flex items-center justify-between gap-3">
        <span id="site-language">{t('account.preferences.language')}</span>
        <Select items={options} value={language} onValueChange={(value) => onLanguageChange(value as LanguageChoice)}>
          <SelectTrigger aria-labelledby="site-language" className="w-select max-sm:w-auto max-sm:min-w-40">
            <SelectValue />
          </SelectTrigger>
          <SelectContent>
            {options.map((option) => (
              <SelectItem key={option.value} value={option.value}>
                {option.label}
              </SelectItem>
            ))}
          </SelectContent>
        </Select>
      </div>
    </Card>
  );
}
