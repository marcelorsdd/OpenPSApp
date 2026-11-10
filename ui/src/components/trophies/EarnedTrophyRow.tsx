import { useTranslation } from 'react-i18next';
import TrophyGlyph from '@/components/trophies/TrophyGlyph';
import CardRow from '@/components/ui/CardRow';
import Cover from '@/components/ui/Cover';
import { useDate } from '@/hooks/useDate';
import type { EarnedTrophy } from '@/types/api';
import { href } from '@/utils/router';

export default function EarnedTrophyRow({ item, showDate = true }: { item: EarnedTrophy; showDate?: boolean }) {
  const { t } = useTranslation();
  const date = useDate();
  const { setId, setName, titleId, trophy } = item;
  return (
    <CardRow to={titleId ? href.game(titleId) : href.set(setId)} icon={<Cover src={trophy.iconUrl} size="w-16 h-16" />} title={trophy.name}>
      <div className="text-sm text-muted truncate">{setName}</div>
      <div className="mt-1 flex items-center text-sm text-muted">
        <TrophyGlyph grade={trophy.grade} className="w-4 h-4" />
        <span className="ml-2">{showDate ? date(trophy.earnedAt) : trophy.grade === 'unknown' ? '' : t(`grade.${trophy.grade}`)}</span>
      </div>
    </CardRow>
  );
}
