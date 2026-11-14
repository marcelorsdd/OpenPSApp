import { useCallback, useEffect, useState } from 'react';
import { type Endpoint, get } from '@/services/api';

export interface ApiState<T> {
  data: T | null;
  error: string | null;
  reload: () => void;
}

interface Loaded<T> {
  endpoint: Endpoint<T> | null;
  data: T | null;
  error: string | null;
}

export function useApi<T>(endpoint: Endpoint<T> | null): ApiState<T> {
  const [loaded, setLoaded] = useState<Loaded<T>>({ endpoint, data: null, error: null });
  const [attempt, setAttempt] = useState(0);
  const reload = useCallback(() => {
    setLoaded((last) => (last.error ? { ...last, error: null } : last));
    setAttempt((n) => n + 1);
  }, []);

  useEffect(() => {
    if (endpoint === null) return;
    let cancelled = false;
    get(endpoint)
      .then((data) => !cancelled && setLoaded({ endpoint, data, error: null }))
      .catch(
        (e: Error) =>
          !cancelled && setLoaded((last) => ({ endpoint, data: last.endpoint === endpoint ? last.data : null, error: e.message })),
      );
    return () => {
      cancelled = true;
    };
  }, [endpoint, attempt]);

  const current = loaded.endpoint === endpoint;
  return { data: current ? loaded.data : null, error: current ? loaded.error : null, reload };
}

export function combine(...states: ApiState<unknown>[]) {
  return {
    error: states.find((s) => s.error)?.error ?? null,
    reload: () => states.forEach((s) => s.reload()),
  };
}
