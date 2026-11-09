import { useTranslation } from 'react-i18next';
import type { EarnedTrophy } from '../api';
import { CardRow, Cover, TrophyGlyph } from '../components/ui';
import { useDate } from '../i18n/useLanguage';
import { href } from '../router';

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
