import { useTranslation } from 'react-i18next';
import TrophyGlyph from '@/components/trophies/TrophyGlyph';
import CardRow from '@/components/ui/CardRow';
import Cover from '@/components/ui/Cover';
import Section from '@/components/ui/Section';
import { useDate } from '@/hooks/useDate';
import type { Trophy } from '@/types/api';

export default function RecentTrophies({ trophies }: { trophies: Trophy[] }) {
  const { t } = useTranslation();
  const date = useDate();
  if (trophies.length === 0) return null;

  return (
    <Section title={t('game.recentTrophies')}>
      <ul className="grid sm:grid-cols-2 gap-x-3">
        {trophies.map((trophy) => (
          <CardRow key={trophy.id} icon={<Cover src={trophy.iconUrl} size="w-16 h-16" />} title={trophy.name}>
            <div className="mt-1 flex items-center text-sm text-muted">
              <TrophyGlyph grade={trophy.grade} className="w-4 h-4" />
              <span className="ml-2">{date(trophy.earnedAt)}</span>
            </div>
          </CardRow>
        ))}
      </ul>
    </Section>
  );
}
