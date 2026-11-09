import { useTranslation } from 'react-i18next';
import { paths, useApi, type RecentTrophy } from '../api';
import { CardRow, Cover, Section, TrophyGlyph } from '../components/ui';
import { useDate } from '../i18n/useLanguage';
import { href } from '../router';

const LIMIT = 6;

export default function RecentTrophies() {
  const { t } = useTranslation();
  const date = useDate();
  const recent = useApi<RecentTrophy[]>(paths.recentTrophies(LIMIT));

  if (!recent.data || recent.data.length === 0) return null;

  return (
    <Section title={t('profile.recentTrophies')}>
      <ul className="grid sm:grid-cols-2 gap-x-3">
        {recent.data.map(({ setId, setName, titleId, trophy }) => (
          <CardRow
            key={`${setId}:${trophy.id}`}
            to={titleId ? href.game(titleId) : href.set(setId)}
            icon={<Cover src={trophy.iconUrl} size="w-16 h-16" />}
            title={trophy.name}
          >
            <div className="text-sm text-muted truncate">{setName}</div>
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
