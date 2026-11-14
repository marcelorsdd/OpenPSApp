import type { DeviceInfo } from '@openpsapp/contract/pairing';
import { describe, expect, it, vi } from 'vitest';
import { ApiError } from '@/lib/api';
import { codeError, displayCode, firstSyncPhase, initialCode, shouldInspectOnLoad } from './describe-link';

vi.mock('@clerk/react', () => ({ getToken: () => Promise.resolve(null) }));

const device = (id: string, lastSyncAt: string | null): DeviceInfo => ({
  id,
  profileName: 'Player',
  createdAt: '2026-11-14T10:00:00Z',
  lastSyncAt,
});

describe('initialCode', () => {
  it('cleans the code from the link like a typed one', () => {
    expect(initialCode('abcd-2345')).toBe('ABCD2345');
    expect(initialCode(null)).toBe('');
    expect(shouldInspectOnLoad('ABCD2345')).toBe(true);
    expect(shouldInspectOnLoad('ABCD')).toBe(false);
    expect(displayCode('ABCD2345')).toBe('ABCD-2345');
  });
});

describe('codeError', () => {
  it('maps server answers to what the code step shows', () => {
    expect(codeError(new ApiError(404, 'invalid_code'))).toBe('invalid');
    expect(codeError(new ApiError(429, 'rate_limited'))).toBe('rateLimited');
    expect(codeError(new ApiError(500, 'internal'))).toBe('failed');
    expect(codeError(new TypeError('Failed to fetch'))).toBe('failed');
  });
});

describe('firstSyncPhase', () => {
  it('ignores the console that was there before the link', () => {
    expect(firstSyncPhase([device('old', '2026-11-14T09:00:00Z')], ['old'])).toBe('waiting');
  });

  it('follows the new console from its token to its first sync', () => {
    expect(firstSyncPhase([], [])).toBe('waiting');
    expect(firstSyncPhase([device('new', null)], ['old'])).toBe('connected');
    expect(firstSyncPhase([device('new', '2026-11-14T10:01:00Z')], ['old'])).toBe('synced');
  });
});
