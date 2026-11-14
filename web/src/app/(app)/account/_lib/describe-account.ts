import type { Profile } from '@openpsapp/contract/api';
import { LANGUAGES } from '@openpsapp/core';
import { isApiError } from '@/lib/api';
import type { AccountTotals, DeleteError, LanguageOption } from '../_types/account';

export function disconnectPath(deviceId: string, purge: boolean): string {
  const path = `/devices/${encodeURIComponent(deviceId)}`;
  return purge ? `${path}?purge=true` : path;
}

export function languageOptions(autoLabel: string): LanguageOption[] {
  return [{ value: 'auto', label: autoLabel }, ...LANGUAGES.map((l) => ({ value: l.code, label: l.label }))];
}

export function accountTotals(profile: Profile | undefined): AccountTotals | null {
  if (!profile) return null;
  return { games: profile.totals.games, trophies: profile.totals.trophies.total };
}

export function deleteError(error: unknown): DeleteError {
  return isApiError(error, 'auth_delete_failed') ? 'loginFailed' : 'failed';
}
