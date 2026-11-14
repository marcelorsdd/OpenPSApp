import type { DeviceInfo } from '@openpsapp/contract/pairing';
import { describe, expect, it } from 'vitest';
import { currentDevice, describeSync } from './sync-status';

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
