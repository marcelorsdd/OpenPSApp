import { useTranslation } from 'react-i18next';
import type { Game, TrophySet } from '../api';
import { Section, Stat } from '../components/ui';
import { formatDuration } from '../format';
import { stats } from '../progress';

export default function StatsCard({ games, sets }: { games: Game[]; sets: TrophySet[] }) {
  const { t } = useTranslation();
  const s = stats(games, sets);
  if (s.startedSets === 0) return null;

  return (
    <Section title={t('stats.title')}>
      <div className="grid sm:grid-cols-3 gap-4">
        <Stat label={t('stats.perTrophy')}>
          {s.minutesPerTrophy === null ? '—' : t('stats.perTrophyValue', { duration: formatDuration(s.minutesPerTrophy * 60) })}
        </Stat>
        <Stat label={t('stats.completion')}>{s.averageCompletion === null ? '—' : `${s.averageCompletion}%`}</Stat>
        <Stat label={t('stats.platinums')}>{t('stats.platinumsValue', { done: s.platinumSets, count: s.startedSets })}</Stat>
      </div>
    </Section>
  );
}
