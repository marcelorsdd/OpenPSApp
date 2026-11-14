import { describe, expect, it } from 'vitest';
import { qrMatrix, qrPath } from '@/utils/qr';

describe('qrMatrix', () => {
  it('builds a square matrix with finder patterns', () => {
    const m = qrMatrix('https://openpsapp.app/#/link?code=ABCD2345');
    expect(m.length).toBeGreaterThanOrEqual(21);
    expect(m.every((row) => row.length === m.length)).toBe(true);
    expect(m[0].slice(0, 7).every(Boolean)).toBe(true);
    expect(m[1][1]).toBe(false);
    expect(m[3][3]).toBe(true);
  });
});

describe('qrPath', () => {
  it('draws one unit square per dark module, offset by the quiet zone', () => {
    expect(
      qrPath(
        [
          [true, false],
          [false, true],
        ],
        4,
      ),
    ).toBe('M4 4h1v1h-1zM5 5h1v1h-1z');
  });
});
