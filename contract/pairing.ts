export interface PairingCodeRequest {
  profileName: string;
}

export interface PairingCodeResponse {
  deviceCode: string;
  userCode: string;
  verificationUri: string;
  expiresIn: number;
  interval: number;
}

export interface PairingConfirmRequest {
  userCode: string;
}

export interface PairingInspectResponse {
  profileName: string;
}

export interface PairingConfirmResponse {
  profileName: string;
}

export interface PairingTokenRequest {
  deviceCode: string;
}

export interface PairingTokenResponse {
  deviceId: string;
  token: string;
}

export interface DeviceInfo {
  id: string;
  profileName: string;
  createdAt: string;
  lastSyncAt: string | null;
}
