import type { ImageSync } from '@openpsapp/contract/api';
import type { DeviceInfo } from '@openpsapp/contract/pairing';
import type { SyncState } from '../_types/sync';

export function currentDevice(devices: DeviceInfo[] | undefined): DeviceInfo | null {
  if (!devices || devices.length === 0) return null;
  return devices.reduce((latest, device) => (device.createdAt > latest.createdAt ? device : latest));
}

export function describeSync(devices: DeviceInfo[] | undefined): SyncState {
  const device = currentDevice(devices);
  if (!device) return { kind: 'none' };
  if (!device.lastSyncAt) return { kind: 'waiting', deviceId: device.id, name: device.profileName };
  return { kind: 'synced', deviceId: device.id, name: device.profileName, at: device.lastSyncAt };
}

export function pendingImages(images: ImageSync | null | undefined): ImageSync | null {
  return images && images.stored < images.total ? images : null;
}

export function avatarUrl(user: { hasImage: boolean; imageUrl: string } | null | undefined): string | null {
  return user?.hasImage ? user.imageUrl : null;
}
