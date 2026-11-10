import { useCallback, useEffect, useState } from 'react';
import { get, type Endpoint } from '@/services/api';

export interface ApiState<T> {
  data: T | null;
  error: string | null;
  reload: () => void;
}

export function useApi<T>(endpoint: Endpoint<T> | null): ApiState<T> {
  const [data, setData] = useState<T | null>(null);
  const [error, setError] = useState<string | null>(null);
  const [attempt, setAttempt] = useState(0);
  const reload = useCallback(() => setAttempt((n) => n + 1), []);

  useEffect(() => {
    if (endpoint === null) return;
    let cancelled = false;
    setError(null);
    get(endpoint)
      .then((body) => !cancelled && setData(body))
      .catch((e: Error) => !cancelled && setError(e.message));
    return () => {
      cancelled = true;
    };
  }, [endpoint, attempt]);

  return { data, error, reload };
}

export function combine(...states: ApiState<unknown>[]) {
  return {
    error: states.find((s) => s.error)?.error ?? null,
    reload: () => states.forEach((s) => s.reload()),
  };
}
