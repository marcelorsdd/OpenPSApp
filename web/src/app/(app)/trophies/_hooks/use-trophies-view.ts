import { useState } from 'react';
import { useStoredState } from '@/hooks/use-stored-state';
import { parseSetSort } from '../_lib/describe-trophies';
import type { SetSort } from '../_types/trophies';

export function useTrophiesView() {
  const [storedSort, setSort] = useStoredState<SetSort>('trophies.sort', 'recent');
  const [incompleteOnly, setIncompleteOnly] = useStoredState('trophies.incomplete', false);
  const [fullTimeline, setFullTimeline] = useState(false);
  return {
    sort: parseSetSort(storedSort),
    setSort,
    incompleteOnly: incompleteOnly === true,
    setIncompleteOnly,
    fullTimeline,
    toggleTimeline: () => setFullTimeline((value) => !value),
  };
}
