export type LinkStep =
  | { kind: 'code' }
  | { kind: 'scan' }
  | { kind: 'confirm'; code: string; profileName: string; replaces: string | null }
  | { kind: 'done'; profileName: string; previousDeviceIds: string[] };

export type CodeError = 'invalid' | 'rateLimited' | 'failed';

export type FirstSyncPhase = 'waiting' | 'connected' | 'synced';

export type CameraStatus = 'starting' | 'scanning' | 'denied' | 'unavailable' | 'failed';

export interface BarcodeResult {
  rawValue?: string;
  text?: string;
}

export interface BarcodeDetectorClass {
  new (options: { formats: string[] }): { detect(source: unknown): Promise<BarcodeResult[]> };
  getSupportedFormats(): Promise<string[]>;
}

export type QrDecoder = (frame: unknown) => Promise<string | null>;

export interface QrDecoderDeps {
  barcodeDetector: { getSupportedFormats(): Promise<string[]>; create(): { detect(source: unknown): Promise<BarcodeResult[]> } } | null;
  loadZxing: () => Promise<(frame: unknown) => Promise<BarcodeResult[]>>;
}
