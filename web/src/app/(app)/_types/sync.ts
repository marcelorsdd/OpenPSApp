export type SyncState =
  { kind: 'none' } | { kind: 'waiting'; deviceId: string; name: string } | { kind: 'synced'; deviceId: string; name: string; at: string };
