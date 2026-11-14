import { afterEach, beforeEach, describe, expect, it, vi } from 'vitest';
import { api, apiDelete, ApiError, apiPost, isApiError } from './api';

const token = vi.hoisted(() => ({ value: null as string | null }));

vi.mock('@clerk/react', () => ({ getToken: () => Promise.resolve(token.value) }));

const calls: { url: string; init: RequestInit }[] = [];

function reply(status: number, body?: unknown) {
  vi.stubGlobal('fetch', (url: string, init: RequestInit) => {
    calls.push({ url, init });
    return Promise.resolve(new Response(body === undefined ? null : JSON.stringify(body), { status }));
  });
}

beforeEach(() => {
  token.value = null;
});

afterEach(() => {
  calls.length = 0;
  vi.unstubAllGlobals();
});

describe('api', () => {
  it('sends the Clerk session token as a bearer token', async () => {
    token.value = 'sess_1';
    reply(200, { ok: true });
    await expect(api('/profile')).resolves.toEqual({ ok: true });
    expect(calls[0].url).toBe('/api/v1/profile');
    expect(new Headers(calls[0].init.headers).get('Authorization')).toBe('Bearer sess_1');
  });

  it('sends no Authorization header when signed out', async () => {
    reply(200, {});
    await api('/health');
    expect(new Headers(calls[0].init.headers).has('Authorization')).toBe(false);
  });

  it('posts JSON bodies', async () => {
    reply(200, { profileName: 'Player', replaces: null });
    await apiPost('/devices/inspect', { userCode: 'ABCD2345' });
    expect(calls[0].init.method).toBe('POST');
    expect(calls[0].init.body).toBe('{"userCode":"ABCD2345"}');
    expect(new Headers(calls[0].init.headers).get('Content-Type')).toBe('application/json');
  });

  it('accepts empty replies', async () => {
    reply(204);
    await expect(apiDelete('/devices/d1?purge=true')).resolves.toBeUndefined();
    expect(calls[0].init.method).toBe('DELETE');
  });

  it('turns errors into ApiError with the server code', async () => {
    reply(404, { error: 'invalid_code' });
    const error = await api('/devices/confirm').catch((e: unknown) => e);
    expect(error).toBeInstanceOf(ApiError);
    expect(error).toMatchObject({ status: 404, code: 'invalid_code' });
    expect(isApiError(error, 'invalid_code')).toBe(true);
    expect(isApiError(error, 'rate_limited')).toBe(false);
  });

  it('keeps a null code when the error body is not JSON', async () => {
    vi.stubGlobal('fetch', () => Promise.resolve(new Response('oops', { status: 502 })));
    await expect(api('/games')).rejects.toMatchObject({ status: 502, code: null });
  });
});
