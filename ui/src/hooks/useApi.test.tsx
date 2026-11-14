import { act, renderHook, waitFor } from '@testing-library/react';
import { afterEach, beforeEach, describe, expect, it, vi } from 'vitest';
import { useApi } from '@/hooks/useApi';
import type { Endpoint } from '@/services/api';

const replies = new Map<string, (value: Response) => void>();

beforeEach(() => {
  vi.stubGlobal(
    'fetch',
    (url: string) =>
      new Promise<Response>((resolve) => {
        replies.set(url, resolve);
      }),
  );
});

afterEach(() => {
  replies.clear();
  vi.unstubAllGlobals();
});

async function reply(url: string, status: number, body: unknown) {
  await act(async () => {
    replies.get(url)?.(new Response(JSON.stringify(body), { status }));
    await new Promise((resolve) => setTimeout(resolve, 10));
  });
}

const game = (titleId: string) => `/api/v1/games/${titleId}` as Endpoint<{ name: string }>;

describe('useApi', () => {
  it('drops the previous data as soon as the endpoint changes', async () => {
    const { result, rerender } = renderHook(({ endpoint }) => useApi(endpoint), { initialProps: { endpoint: game('A') } });
    await reply(game('A'), 200, { name: 'A' });
    await waitFor(() => expect(result.current.data).toEqual({ name: 'A' }));

    rerender({ endpoint: game('B') });
    expect(result.current.data).toBeNull();

    await reply(game('B'), 200, { name: 'B' });
    await waitFor(() => expect(result.current.data).toEqual({ name: 'B' }));
  });

  it('ignores a late reply for an endpoint it no longer shows', async () => {
    const { result, rerender } = renderHook(({ endpoint }) => useApi(endpoint), { initialProps: { endpoint: game('A') } });
    rerender({ endpoint: game('B') });
    await reply(game('B'), 200, { name: 'B' });
    await waitFor(() => expect(result.current.data).toEqual({ name: 'B' }));

    await reply(game('A'), 200, { name: 'A' });
    expect(result.current.data).toEqual({ name: 'B' });
  });

  it('keeps the data while reloading and clears an error on retry', async () => {
    const { result } = renderHook(() => useApi(game('A')));
    await reply(game('A'), 500, { error: 'internal' });
    await waitFor(() => expect(result.current.error).toBe('HTTP 500'));

    act(() => result.current.reload());
    expect(result.current.error).toBeNull();
    await reply(game('A'), 200, { name: 'A' });
    await waitFor(() => expect(result.current.data).toEqual({ name: 'A' }));

    act(() => result.current.reload());
    expect(result.current.data).toEqual({ name: 'A' });
  });

  it('stays empty without an endpoint', () => {
    const { result } = renderHook(() => useApi(null));
    expect(result.current).toMatchObject({ data: null, error: null });
    expect(replies.size).toBe(0);
  });
});
