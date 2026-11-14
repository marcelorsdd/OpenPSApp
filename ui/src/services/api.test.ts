import { afterEach, describe, expect, it, vi } from 'vitest';
import { ApiError, cloudAction, endpoints, get, setGameHidden } from '@/services/api';

const calls: { url: string; init: RequestInit | undefined }[] = [];

function stubFetch(status: number, body?: unknown) {
  vi.stubGlobal('fetch', (url: string, init?: RequestInit) => {
    calls.push({ url, init });
    return Promise.resolve(new Response(body === undefined ? null : JSON.stringify(body), { status }));
  });
}

afterEach(() => {
  calls.length = 0;
  vi.unstubAllGlobals();
});

describe('console actions', () => {
  it('mark hide and unhide so other sites need a preflight', async () => {
    stubFetch(200, { titleId: 'PPSA/1', hidden: true });
    await setGameHidden('PPSA/1', true);
    await setGameHidden('PPSA/1', false);
    expect(calls.map((c) => c.url)).toEqual(['/api/v1/games/PPSA%2F1/hide', '/api/v1/games/PPSA%2F1/unhide']);
    for (const { init } of calls) {
      expect(init?.method).toBe('POST');
      expect(new Headers(init?.headers).get('X-OpenPSApp')).toBe('1');
    }
  });

  it('mark cloud actions the same way', async () => {
    stubFetch(200, { configured: true, local: true, linked: false });
    await cloudAction('link');
    expect(calls[0].url).toBe('/api/v1/cloud/link');
    expect(calls[0].init?.method).toBe('POST');
    expect(new Headers(calls[0].init?.headers).get('X-OpenPSApp')).toBe('1');
  });
});

describe('request', () => {
  it('reads the data with a plain GET', async () => {
    stubFetch(200, { ok: true });
    await expect(get(endpoints.health)).resolves.toEqual({ ok: true });
    expect(calls[0].init?.method).toBeUndefined();
  });

  it('accepts empty replies', async () => {
    stubFetch(204);
    await expect(cloudAction('sync')).resolves.toBeUndefined();
  });

  it('exposes the server error code', async () => {
    stubFetch(409, { error: 'not_linked' });
    const error = await cloudAction('unlink').catch((e: unknown) => e);
    expect(error).toBeInstanceOf(ApiError);
    expect(error).toMatchObject({ status: 409, code: 'not_linked' });
  });
});
