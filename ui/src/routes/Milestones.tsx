import { useTranslation } from 'react-i18next';
import type { EarnedTrophy, TrophySet } from '../api';
import { CardRow, Cover, Section } from '../components/ui';
import { useDate } from '../i18n/useLanguage';
import { milestones, nextMilestone, type Milestone } from '../progress';
import { href } from '../router';

export default function Milestones({ earned, sets }: { earned: EarnedTrophy[]; sets: TrophySet[] }) {
  const { t } = useTranslation();
  const date = useDate();
  const latest = milestones(earned, sets).slice(0, 3);
  const next = nextMilestone(earned.length);

  const text = (m: Milestone): string => {
    switch (m.kind) {
      case 'firstTrophy':
        return t('milestones.firstTrophy');
      case 'firstGrade':
        return t(`milestones.firstGrade.${m.grade}`);
      case 'trophyCount':
        return t('milestones.trophyCount', { n: m.count });
      case 'platinumCount':
        return t('milestones.platinumCount', { n: m.count });
      case 'complete':
        return t('milestones.complete', { game: m.item.setName });
    }
  };

  return (
    <Section title={t('milestones.title')}>
      <ul>
        {latest.map((m) => (
          <CardRow
            key={`${m.kind}:${m.item.setId}:${m.item.trophy.id}`}
            to={m.item.titleId ? href.game(m.item.titleId) : href.set(m.item.setId)}
            icon={<Cover src={m.item.trophy.iconUrl} size="w-16 h-16" />}
            title={text(m)}
          >
            <div className="text-sm text-muted truncate">
              {m.item.setName} · {date(m.item.trophy.earnedAt)}
            </div>
          </CardRow>
        ))}
      </ul>
      <p className="text-muted">{t('milestones.next', { n: next.count, remaining: next.remaining })}</p>
    </Section>
  );
}
