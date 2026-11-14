import type { Profile } from '@openpsapp/contract/api';
import { describe, expect, it, vi } from 'vitest';
import { ApiError } from '@/lib/api';
import { accountTotals, deleteError, disconnectPath, languageOptions } from './describe-account';

vi.mock('@clerk/react', () => ({ getToken: () => Promise.resolve(null) }));

describe('disconnectPath', () => {
  it('asks the server to purge the data only when chosen', () => {
    expect(disconnectPath('d1', false)).toBe('/devices/d1');
    expect(disconnectPath('d1', true)).toBe('/devices/d1?purge=true');
    expect(disconnectPath('a/b', false)).toBe('/devices/a%2Fb');
  });
});

describe('languageOptions', () => {
  it('starts with the automatic choice', () => {
    expect(languageOptions('Auto').map((o) => o.value)).toEqual(['auto', 'pt-BR', 'en', 'es']);
  });
});

describe('accountTotals', () => {
  it('counts games and every trophy on the account', () => {
    const profile: Profile = {
      user: { id: 'a', name: 'Player' },
      totals: { games: 3, playtimeSeconds: 1, trophies: { platinum: 0, gold: 0, silver: 3, bronze: 18, earned: 21, total: 78 } },
    };
    expect(accountTotals(profile)).toEqual({ games: 3, trophies: 78 });
    expect(accountTotals(undefined)).toBeNull();
  });
});

describe('deleteError', () => {
  it('tells a removed account whose login survived from a plain failure', () => {
    expect(deleteError(new ApiError(502, 'auth_delete_failed'))).toBe('loginFailed');
    expect(deleteError(new ApiError(502, null))).toBe('failed');
    expect(deleteError(new ApiError(500, 'internal'))).toBe('failed');
    expect(deleteError(new TypeError('Failed to fetch'))).toBe('failed');
  });
});
