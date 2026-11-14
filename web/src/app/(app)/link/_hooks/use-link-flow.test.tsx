import '@/i18n';
import type { DeviceInfo } from '@openpsapp/contract/pairing';
import { QueryClientProvider } from '@tanstack/react-query';
import { act, renderHook, waitFor } from '@testing-library/react';
import type { ReactNode } from 'react';
import { MemoryRouter, useSearchParams } from 'react-router';
import { afterEach, beforeEach, describe, expect, it, vi } from 'vitest';
import { createQueryClient } from '@/lib/query-client';
import { queryKeys } from '@/lib/query-keys';
import { FIRST_SYNC_POLL_MS } from '../_lib/describe-link';
import { useFirstSync } from './use-first-sync';
import { useLinkFlow } from './use-link-flow';

const toast = vi.hoisted(() => ({ success: vi.fn(), error: vi.fn() }));

vi.mock('@clerk/react', () => ({ getToken: () => Promise.resolve('sess_1') }));
vi.mock('sonner', () => ({ toast }));

const device = (id: string, lastSyncAt: string | null): DeviceInfo => ({
  id,
  profileName: 'tester',
  createdAt: '2026-11-14T10:00:00Z',
  lastSyncAt,
});

interface Reply {
  status: number;
  body: unknown;
}

const OLD = device('old', '2026-11-13T10:00:00Z');
const requests: string[] = [];
let confirmReply: Reply;
let afterConfirm: DeviceInfo[][];

function devicesReply(): Reply {
  if (!requests.includes('POST /api/v1/devices/confirm')) return { status: 200, body: [OLD] };
  return { status: 200, body: afterConfirm.length > 1 ? afterConfirm.shift() : afterConfirm[0] };
}

beforeEach(() => {
  vi.useFakeTimers({ shouldAdvanceTime: true });
  confirmReply = { status: 200, body: { profileName: 'tester' } };
  afterConfirm = [
    [OLD, device('new', null)],
    [OLD, device('new', '2026-11-14T10:01:00Z')],
  ];
  vi.stubGlobal('fetch', (url: string, init?: RequestInit) => {
    const request = `${init?.method ?? 'GET'} ${url}`;
    requests.push(request);
    const reply =
      request === 'POST /api/v1/devices/inspect'
        ? { status: 200, body: { profileName: 'tester', replaces: 'old' } }
        : request === 'POST /api/v1/devices/confirm'
          ? confirmReply
          : devicesReply();
    return Promise.resolve(new Response(JSON.stringify(reply.body), { status: reply.status }));
  });
});

afterEach(() => {
  requests.length = 0;
  vi.clearAllMocks();
  vi.unstubAllGlobals();
  vi.useRealTimers();
});

function renderLinkPage(entry: string) {
  const client = createQueryClient();
  const invalidate = vi.spyOn(client, 'invalidateQueries');
  const wrapper = ({ children }: { children: ReactNode }) => (
    <QueryClientProvider client={client}>
      <MemoryRouter initialEntries={[entry]}>{children}</MemoryRouter>
    </QueryClientProvider>
  );
  const view = renderHook(
    () => {
      const flow = useLinkFlow();
      const sync = useFirstSync(flow.step.kind === 'done' ? flow.step.previousDeviceIds : null);
      const [params] = useSearchParams();
      return { flow, sync, code: params.get('code') };
    },
    { wrapper },
  );
  return { ...view, invalidate };
}

const nextPoll = () => act(() => vi.advanceTimersByTimeAsync(FIRST_SYNC_POLL_MS));

describe('useLinkFlow', () => {
  it('inspects the code from the link, confirms it and follows the new console to its first sync', async () => {
    const { result, invalidate } = renderLinkPage('/link?code=abcd-2345');
    await waitFor(() =>
      expect(result.current.flow.step).toEqual({ kind: 'confirm', code: 'ABCD2345', profileName: 'tester', replaces: 'old' }),
    );

    act(() => result.current.flow.confirm());
    await waitFor(() => expect(result.current.flow.step).toEqual({ kind: 'done', profileName: 'tester', previousDeviceIds: ['old'] }));
    expect(requests.filter((request) => request.startsWith('POST'))).toEqual([
      'POST /api/v1/devices/inspect',
      'POST /api/v1/devices/confirm',
    ]);
    expect(result.current.code).toBeNull();
    expect(toast.success).toHaveBeenCalledTimes(1);
    expect(result.current.sync).toEqual({ phase: 'waiting', step: 1, steps: 3 });

    await nextPoll();
    await waitFor(() => expect(result.current.sync.phase).toBe('connected'));
    expect(invalidate).not.toHaveBeenCalledWith({ queryKey: queryKeys.library });

    await nextPoll();
    await waitFor(() => expect(result.current.sync).toEqual({ phase: 'synced', step: 3, steps: 3 }));
    expect(invalidate).toHaveBeenCalledWith({ queryKey: queryKeys.library });

    const polls = requests.length;
    await nextPoll();
    await nextPoll();
    expect(requests).toHaveLength(polls);
  });

  it('goes back to the code with the reason when the confirmation fails', async () => {
    confirmReply = { status: 404, body: { error: 'invalid_code' } };
    const { result } = renderLinkPage('/link?code=ABCD2345');
    await waitFor(() => expect(result.current.flow.step.kind).toBe('confirm'));

    act(() => result.current.flow.confirm());
    await waitFor(() => expect(result.current.flow.step).toEqual({ kind: 'code' }));
    expect(result.current.flow.code.rootError).toBe('invalid');
    expect(result.current.code).toBe('ABCD2345');
  });

  it('accepts only scans of a pairing code or of this site link page', async () => {
    const { result } = renderLinkPage('/link');
    act(() => result.current.flow.scan());
    expect(result.current.flow.step).toEqual({ kind: 'scan' });

    let accepted = true;
    act(() => {
      accepted = result.current.flow.acceptScan('https://example.com/link?code=ABCD2345');
    });
    expect(accepted).toBe(false);
    expect(result.current.flow.step).toEqual({ kind: 'scan' });

    act(() => {
      accepted = result.current.flow.acceptScan(`${window.location.origin}/link?code=ABCD2345`);
    });
    expect(accepted).toBe(true);
    await waitFor(() => expect(result.current.flow.step.kind).toBe('confirm'));
    expect(requests.filter((request) => request.startsWith('POST'))).toEqual(['POST /api/v1/devices/inspect']);
  });
});
