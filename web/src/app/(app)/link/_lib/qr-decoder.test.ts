import { describe, expect, it, vi } from 'vitest';
import type { QrDecoderDeps } from '../_types/link';
import { cameraStatusFor, centerSquare, createQrDecoder, firstValue } from './qr-decoder';

function deps(formats: string[] | null) {
  const detect = vi.fn(() => Promise.resolve([{ rawValue: 'from-detector' }]));
  const zxing = vi.fn(() => Promise.resolve([{ text: 'from-zxing' }]));
  const loadZxing = vi.fn(() => Promise.resolve(zxing));
  const value: QrDecoderDeps = {
    barcodeDetector: formats ? { getSupportedFormats: () => Promise.resolve(formats), create: () => ({ detect }) } : null,
    loadZxing,
  };
  return { value, detect, zxing, loadZxing };
}

describe('createQrDecoder', () => {
  it('uses BarcodeDetector when it reads QR codes', async () => {
    const d = deps(['ean_13', 'qr_code']);
    const decode = await createQrDecoder(d.value);
    expect(await decode('frame')).toBe('from-detector');
    expect(d.loadZxing).not.toHaveBeenCalled();
  });

  it('falls back to the zxing reader without BarcodeDetector or QR support', async () => {
    for (const formats of [null, ['ean_13']]) {
      const d = deps(formats);
      const decode = await createQrDecoder(d.value);
      expect(await decode('frame')).toBe('from-zxing');
      expect(d.zxing).toHaveBeenCalledWith('frame');
    }
  });
});

describe('firstValue', () => {
  it('returns the first non-empty value', () => {
    expect(firstValue([])).toBeNull();
    expect(firstValue([{ rawValue: '' }, { text: 'B' }])).toBe('B');
  });
});

describe('cameraStatusFor', () => {
  it('tells a denied permission from a missing camera', () => {
    expect(cameraStatusFor({ name: 'NotAllowedError' })).toBe('denied');
    expect(cameraStatusFor({ name: 'NotFoundError' })).toBe('unavailable');
    expect(cameraStatusFor({ name: 'OverconstrainedError' })).toBe('unavailable');
    expect(cameraStatusFor(new Error('boom'))).toBe('failed');
    expect(cameraStatusFor(null)).toBe('failed');
  });
});

describe('centerSquare', () => {
  it('crops the middle square of the frame', () => {
    expect(centerSquare(1280, 720)).toEqual({ x: 280, y: 0, size: 720 });
    expect(centerSquare(720, 1280)).toEqual({ x: 0, y: 280, size: 720 });
  });
});
