import { useState } from 'react';
import { paths, useApi, type Trophy } from '../api';
import { CardRow, Cover, ErrorBox, Loading, Pill, TrophyGlyph } from '../components/ui';
import { filterTrophies, formatDate, GRADE_LABEL, sortTrophies, trophyView, type TrophyFilter, type TrophySort } from '../format';
import { useStoredState } from '../storage';

const SORTS: { value: TrophySort; label: string }[] = [
  { value: 'default', label: 'Padrão' },
  { value: 'earned', label: 'Data de conquista' },
  { value: 'grade', label: 'Tipo de troféu' },
];

export default function TrophySetSection({ setId }: { setId: string }) {
  const trophies = useApi<Trophy[]>(paths.trophies(setId));
  const [filter, setFilter] = useStoredState<TrophyFilter>('trophies.filter', 'all');
  const [sort, setSort] = useStoredState<TrophySort>('trophies.sort', 'default');
  const [showHidden, setShowHidden] = useStoredState('trophies.showHidden', false);
  const [revealed, setRevealed] = useState<Set<number>>(new Set());

  if (trophies.error) return <ErrorBox message={trophies.error} onRetry={trophies.reload} />;
  if (!trophies.data) return <Loading />;

  const list = trophies.data;
  const earned = list.filter((t) => t.earned).length;
  const hiddenLocked = list.filter((t) => t.hidden && !t.earned).length;
  const shown = sortTrophies(filterTrophies(list, filter), sort);

  return (
    <>
      <div className="flex flex-wrap mt-10 -mx-1">
        <Pill active={filter === 'all'} onClick={() => setFilter('all')}>{`Todos (${list.length})`}</Pill>
        <Pill active={filter === 'earned'} onClick={() => setFilter('earned')}>{`Conquistados (${earned})`}</Pill>
        <Pill active={filter === 'locked'} onClick={() => setFilter('locked')}>
          {`Não conquistados (${list.length - earned})`}
        </Pill>
      </div>

      <div className="flex flex-wrap items-center mt-4 -mx-1">
        <span className="mx-1 text-muted">Ordenar:</span>
        {SORTS.map((s) => (
          <Pill key={s.value} active={sort === s.value} onClick={() => setSort(s.value)}>
            {s.label}
          </Pill>
        ))}
        {hiddenLocked > 0 && (
          <Pill active={showHidden} onClick={() => setShowHidden(!showHidden)}>
            {showHidden ? 'Esconder ocultos' : `Mostrar ocultos (${hiddenLocked})`}
          </Pill>
        )}
      </div>

      <ul className="mt-6">
        {shown.map((t) => (
          <TrophyRow
            key={t.id}
            trophy={t}
            revealed={showHidden || revealed.has(t.id)}
            onReveal={() => setRevealed((prev) => new Set(prev).add(t.id))}
          />
        ))}
      </ul>
      {shown.length === 0 && <p className="mt-6 text-muted">Nenhum troféu neste filtro.</p>}
    </>
  );
}

function TrophyRow({ trophy, revealed, onReveal }: { trophy: Trophy; revealed: boolean; onReveal: () => void }) {
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
        <span className="ml-2 text-sm">{trophy.grade === 'unknown' ? '' : GRADE_LABEL[trophy.grade]}</span>
      </div>
      <div className="mt-1 text-sm text-muted">{trophy.earned ? formatDate(trophy.earnedAt) : 'Bloqueado'}</div>
    </>
  );
  return (
    <CardRow icon={icon} title={view.name} aside={aside}>
      <div className="text-muted mt-1 whitespace-normal">{view.description}</div>
      {view.masked && (
        <button onClick={onReveal} className="mt-3 px-4 py-2 rounded-lg border border-line text-sm hover:bg-raised">
          Revelar
        </button>
      )}
    </CardRow>
  );
}
