import { useTranslation } from 'react-i18next';
import { NumberedSteps } from '@/components/numbered-steps';

export function CodeHelp() {
  const { t } = useTranslation();
  return (
    <aside className="basis-aside max-md:hidden">
      <h2 className="text-body font-semibold">{t('link.help.title')}</h2>
      <NumberedSteps className="mt-3 gap-3.5" steps={[t('link.help.one'), t('link.help.two'), t('link.help.three')]} />
    </aside>
  );
}
