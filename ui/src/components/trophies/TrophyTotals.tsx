import { useTranslation } from 'react-i18next';
import GradeCounts from '@/components/trophies/GradeCounts';
import Stat from '@/components/ui/Stat';
import type { Profile } from '@/types/api';

export default function TrophyTotals({ label, trophies }: { label: string; trophies: Profile['totals']['trophies'] }) {
  const { t } = useTranslation();
  return (
    <Stat label={label}>
      <div>
        {trophies.earned}
        <span className="text-muted text-lg font-normal"> {t('common.outOf', { total: trophies.total })}</span>
      </div>
      <div className="mt-3 text-base font-normal">
        <GradeCounts counts={trophies} />
      </div>
    </Stat>
  );
}
