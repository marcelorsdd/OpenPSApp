import { useTranslation } from 'react-i18next';
import TrophyGlyph from '@/components/trophies/TrophyGlyph';
import type { Grade, GradeCount } from '@/types/api';
import { GRADES } from '@/utils/trophies';

export default function GradeCounts({ counts }: { counts: Record<Grade, GradeCount | number> }) {
  const { t } = useTranslation();
  return (
    <div className="flex flex-wrap -mx-2">
      {GRADES.map((g) => {
        const c = counts[g];
        return (
          <span key={g} className="flex items-center mx-2" title={t(`grade.${g}`)}>
            <TrophyGlyph grade={g} />
            <span className="ml-1 tabular-nums">{typeof c === 'number' ? c : `${c.earned}/${c.total}`}</span>
          </span>
        );
      })}
    </div>
  );
}
