import type { DeviceInfo } from '@openpsapp/contract/pairing';
import { describe, expect, it } from 'vitest';
import { avatarUrl, currentDevice, describeSync, pendingImages } from './sync-status';

const device = (id: string, createdAt: string, lastSyncAt: string | null): DeviceInfo => ({
  id,
  profileName: `P${id}`,
  createdAt,
  lastSyncAt,
});

describe('describeSync', () => {
  it('says when no console is connected', () => {
    expect(describeSync(undefined)).toEqual({ kind: 'none' });
    expect(describeSync([])).toEqual({ kind: 'none' });
  });

  it('waits for the first sync of a new console', () => {
    expect(describeSync([device('1', '2026-11-14T10:00:00Z', null)])).toEqual({ kind: 'waiting', deviceId: '1', name: 'P1' });
  });

  it('reports the last sync of the newest console', () => {
    const devices = [
      device('1', '2026-11-01T10:00:00Z', '2026-11-02T10:00:00Z'),
      device('2', '2026-11-14T10:00:00Z', '2026-11-14T16:37:00Z'),
    ];
    expect(currentDevice(devices)?.id).toBe('2');
    expect(describeSync(devices)).toEqual({ kind: 'synced', deviceId: '2', name: 'P2', at: '2026-11-14T16:37:00Z' });
  });
});

describe('pendingImages', () => {
  it('reports images only while some are still on their way', () => {
    expect(pendingImages({ total: 83, stored: 16 })).toEqual({ total: 83, stored: 16 });
    expect(pendingImages({ total: 83, stored: 83 })).toBeNull();
    expect(pendingImages({ total: 0, stored: 0 })).toBeNull();
    expect(pendingImages(undefined)).toBeNull();
  });
});

describe('avatarUrl', () => {
  it('uses the sign-in picture only when the account has one', () => {
    expect(avatarUrl({ hasImage: true, imageUrl: 'https://img.clerk.com/a' })).toBe('https://img.clerk.com/a');
    expect(avatarUrl({ hasImage: false, imageUrl: 'https://img.clerk.com/default' })).toBeNull();
    expect(avatarUrl(null)).toBeNull();
  });
});
