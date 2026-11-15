import type { DeviceInfo } from '@openpsapp/contract/pairing';
import { formatUserCode, isUserCode, sanitizeTypedCode } from '@openpsapp/core';
import { isApiError } from '@/lib/api';
import type { CodeError, FirstSyncPhase } from '../_types/link';

export const FIRST_SYNC_POLL_MS = 3000;
export const FIRST_SYNC_STEPS: FirstSyncPhase[] = ['waiting', 'connected', 'synced'];

export function linkCodeParam(search: string, hash: string): string | null {
  return new URLSearchParams(hash.replace(/^#/, '')).get('code') ?? new URLSearchParams(search).get('code');
}

export function initialCode(param: string | null): string {
  return sanitizeTypedCode(param ?? '');
}

export function shouldInspectOnLoad(code: string): boolean {
  return isUserCode(code);
}

export function codeError(error: unknown): CodeError {
  if (isApiError(error, 'invalid_code') || isApiError(error, 'invalid_body')) return 'invalid';
  if (isApiError(error) && error.status === 429) return 'rateLimited';
  return 'failed';
}

export function displayCode(code: string): string {
  return formatUserCode(code);
}

export function firstSyncPhase(devices: DeviceInfo[] | undefined, previousIds: string[]): FirstSyncPhase {
  const fresh = (devices ?? []).filter((device) => !previousIds.includes(device.id));
  if (fresh.some((device) => device.lastSyncAt)) return 'synced';
  return fresh.length > 0 ? 'connected' : 'waiting';
}
