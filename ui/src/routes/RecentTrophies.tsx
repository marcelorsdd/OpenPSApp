import { useTranslation } from 'react-i18next';
import type { EarnedTrophy } from '../api';
import { Section } from '../components/ui';
import EarnedTrophyRow from './EarnedTrophyRow';

export default function RecentTrophies({ items }: { items: EarnedTrophy[] }) {
  const { t } = useTranslation();
  if (items.length === 0) return null;

  return (
    <Section title={t('profile.recentTrophies')}>
      <ul className="grid sm:grid-cols-2 gap-x-3">
        {items.map((item) => (
          <EarnedTrophyRow key={`${item.setId}:${item.trophy.id}`} item={item} />
        ))}
      </ul>
    </Section>
  );
}
