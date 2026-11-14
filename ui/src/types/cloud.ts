export interface CloudPairing {
  userCode: string;
  verificationUri: string;
  expiresIn: number;
}

export interface CloudStatus {
  configured: boolean;
  local: boolean;
  linked: boolean;
  syncing?: boolean;
  profileName?: string | null;
  lastSyncAt?: string | null;
  lastError?: string | null;
  pairing?: CloudPairing | Pick<CloudPairing, 'expiresIn'> | null;
}
