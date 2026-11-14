import '@/i18n';
import { QueryClientProvider } from '@tanstack/react-query';
import { act, renderHook, waitFor } from '@testing-library/react';
import type { ReactNode } from 'react';
import { afterEach, describe, expect, it, vi } from 'vitest';
import { createQueryClient } from '@/lib/query-client';
import { useDeleteAccount } from './use-delete-account';

const clerk = vi.hoisted(() => ({ signOut: vi.fn(() => Promise.resolve()) }));
const toast = vi.hoisted(() => ({ success: vi.fn(), error: vi.fn() }));

vi.mock('@clerk/react', () => ({ getToken: () => Promise.resolve('sess_1'), useClerk: () => clerk }));
vi.mock('sonner', () => ({ toast }));

function reply(...responses: { status: number; body?: unknown }[]) {
  const fetch = vi.fn((_url: string, _init?: RequestInit) => {
    const next = responses.shift() ?? { status: 500 };
    return Promise.resolve(new Response(next.body === undefined ? null : JSON.stringify(next.body), { status: next.status }));
  });
  vi.stubGlobal('fetch', fetch);
  return fetch;
}

function renderDeleteAccount() {
  const client = createQueryClient();
  const wrapper = ({ children }: { children: ReactNode }) => <QueryClientProvider client={client}>{children}</QueryClientProvider>;
  return renderHook(() => useDeleteAccount(), { wrapper });
}

afterEach(() => {
  vi.clearAllMocks();
  vi.unstubAllGlobals();
});

describe('useDeleteAccount', () => {
  it('says the data is gone but the login stays, and lets the user retry', async () => {
    const fetch = reply({ status: 502, body: { error: 'auth_delete_failed' } }, { status: 204 });
    const { result } = renderDeleteAccount();
    act(() => result.current.setOpen(true));

    act(() => result.current.confirm());
    await waitFor(() => expect(toast.error).toHaveBeenCalledWith('Your data was deleted, but the login could not be removed. Try again.'));
    expect(result.current.open).toBe(true);
    expect(clerk.signOut).not.toHaveBeenCalled();

    act(() => result.current.confirm());
    await waitFor(() => expect(clerk.signOut).toHaveBeenCalledWith({ redirectUrl: '/' }));
    expect(toast.success).toHaveBeenCalledWith('Account deleted');
    expect(result.current.open).toBe(false);
    expect(fetch).toHaveBeenCalledTimes(2);
    expect(fetch.mock.calls.map(([url, init]) => [url, init?.method])).toEqual([
      ['/api/v1/me', 'DELETE'],
      ['/api/v1/me', 'DELETE'],
    ]);
  });

  it('keeps the generic message for other failures', async () => {
    reply({ status: 500, body: { error: 'internal' } });
    const { result } = renderDeleteAccount();
    act(() => result.current.confirm());
    await waitFor(() => expect(toast.error).toHaveBeenCalledWith('Could not delete the account. Try again.'));
  });
});
