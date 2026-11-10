import { useState } from 'react';
import { useTranslation } from 'react-i18next';
import RecentTrophies from '@/components/trophies/RecentTrophies';
import TrophyGlyph from '@/components/trophies/TrophyGlyph';
import CardRow from '@/components/ui/CardRow';
import Cover from '@/components/ui/Cover';
import ErrorBox from '@/components/ui/ErrorBox';
import Loading from '@/components/ui/Loading';
import Pill from '@/components/ui/Pill';
import { useApi } from '@/hooks/useApi';
import { useDate } from '@/hooks/useDate';
import { useStoredState } from '@/hooks/useStoredState';
import { endpoints } from '@/services/api';
import type { Trophy } from '@/types/api';
import type { TrophyFilter, TrophySort } from '@/types/trophies';
import { filterTrophies, recentlyEarned, sortTrophies, trophyView } from '@/utils/trophies';

const SORTS: TrophySort[] = ['default', 'earned', 'grade'];
const RECENT = 6;

export default function TrophySetSection({ setId }: { setId: string }) {
  const { t } = useTranslation();
  const trophies = useApi(endpoints.trophies(setId));
  const [filter, setFilter] = useStoredState<TrophyFilter>('trophies.filter', 'all');
  const [sort, setSort] = useStoredState<TrophySort>('trophies.sort', 'default');
  const [showHidden, setShowHidden] = useStoredState('trophies.showHidden', false);
  const [revealed, setRevealed] = useState<Set<number>>(new Set());

  if (trophies.error) return <ErrorBox message={trophies.error} onRetry={trophies.reload} />;
  if (!trophies.data) return <Loading />;

  const list = trophies.data;
  const earned = list.filter((tr) => tr.earned).length;
  const hiddenLocked = list.filter((tr) => tr.hidden && !tr.earned).length;
  const shown = sortTrophies(filterTrophies(list, filter), sort);
  const counts: Record<TrophyFilter, number> = { all: list.length, earned, locked: list.length - earned };

  return (
    <>
      <RecentTrophies trophies={recentlyEarned(list, RECENT)} />

      <div className="flex flex-wrap mt-10 -mx-1">
        {(['all', 'earned', 'locked'] as TrophyFilter[]).map((f) => (
          <Pill key={f} active={filter === f} onClick={() => setFilter(f)}>
            {t(`trophyList.filter.${f}`, { count: counts[f] })}
          </Pill>
        ))}
      </div>

      <div className="flex flex-wrap items-center mt-4 -mx-1">
        <span className="mx-1 text-muted">{t('trophyList.sortBy')}</span>
        {SORTS.map((s) => (
          <Pill key={s} active={sort === s} onClick={() => setSort(s)}>
            {t(`trophyList.sort.${s}`)}
          </Pill>
        ))}
        {hiddenLocked > 0 && (
          <Pill active={showHidden} onClick={() => setShowHidden(!showHidden)}>
            {showHidden ? t('trophyList.hideHidden') : t('trophyList.showHidden', { count: hiddenLocked })}
          </Pill>
        )}
      </div>

      <ul className="mt-6">
        {shown.map((tr) => (
          <TrophyRow
            key={tr.id}
            trophy={tr}
            revealed={showHidden || revealed.has(tr.id)}
            onReveal={() => setRevealed((prev) => new Set(prev).add(tr.id))}
          />
        ))}
      </ul>
      {shown.length === 0 && <p className="mt-6 text-muted">{t('trophyList.emptyFilter')}</p>}
    </>
  );
}

function TrophyRow({ trophy, revealed, onReveal }: { trophy: Trophy; revealed: boolean; onReveal: () => void }) {
  const { t } = useTranslation();
  const date = useDate();
  const view = trophyView(trophy, revealed);
  const icon = (
    <div className={trophy.earned ? '' : 'opacity-40'}>
      {view.iconUrl ? (
        <Cover src={view.iconUrl} size="w-20 h-20" />
      ) : (
        <div className="w-20 h-20 rounded-xl bg-raised flex items-center justify-center text-3xl text-muted">?</div>
      )}
    </div>
  );
  const aside = (
    <>
      <div className="flex items-center justify-end">
        <TrophyGlyph grade={trophy.grade} className="w-6 h-6" />
        <span className="ml-2 text-sm">{trophy.grade === 'unknown' ? '' : t(`grade.${trophy.grade}`)}</span>
      </div>
      <div className="mt-1 text-sm text-muted">{trophy.earned ? date(trophy.earnedAt) : t('trophyList.locked')}</div>
    </>
  );
  return (
    <CardRow icon={icon} title={view.masked ? t('trophy.hiddenName') : view.name} aside={aside}>
      <div className="text-muted mt-1 whitespace-normal">{view.masked ? t('trophy.hiddenDescription') : view.description}</div>
      {view.masked && (
        <button onClick={onReveal} className="mt-3 px-4 py-2 rounded-lg border border-line text-sm hover:bg-raised">
          {t('trophyList.reveal')}
        </button>
      )}
    </CardRow>
  );
}
