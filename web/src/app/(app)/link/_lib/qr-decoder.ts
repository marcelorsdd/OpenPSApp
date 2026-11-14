import type { BarcodeResult, CameraStatus, QrDecoder, QrDecoderDeps } from '../_types/link';

export const SCAN_INTERVAL_MS = 250;

export function firstValue(results: BarcodeResult[]): string | null {
  for (const result of results) {
    const value = result.rawValue ?? result.text;
    if (value) return value;
  }
  return null;
}

export async function createQrDecoder(deps: QrDecoderDeps): Promise<QrDecoder> {
  if (deps.barcodeDetector) {
    const formats = await deps.barcodeDetector.getSupportedFormats().catch((): string[] => []);
    if (formats.includes('qr_code')) {
      const detector = deps.barcodeDetector.create();
      return async (frame) => firstValue(await detector.detect(frame));
    }
  }
  const read = await deps.loadZxing();
  return async (frame) => firstValue(await read(frame));
}

export function cameraStatusFor(error: unknown): CameraStatus {
  const name = (error as { name?: unknown } | null)?.name;
  if (name === 'NotAllowedError' || name === 'SecurityError' || name === 'PermissionDeniedError') return 'denied';
  if (name === 'NotFoundError' || name === 'OverconstrainedError' || name === 'DevicesNotFoundError') return 'unavailable';
  return 'failed';
}

export function centerSquare(width: number, height: number): { x: number; y: number; size: number } {
  const size = Math.min(width, height);
  return { x: Math.floor((width - size) / 2), y: Math.floor((height - size) / 2), size };
}
