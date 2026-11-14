import { afterEach, describe, expect, it, vi } from 'vitest';
import { readStored, writeStored } from './storage';

function stubStorage(items: Record<string, string>) {
  vi.stubGlobal('window', {
    localStorage: {
      getItem: (key: string) => items[key] ?? null,
      setItem: (key: string, value: string) => {
        items[key] = value;
      },
    },
  });
}

afterEach(() => {
  vi.unstubAllGlobals();
});

describe('readStored', () => {
  it('reads what writeStored saved', () => {
    const items: Record<string, string> = {};
    stubStorage(items);
    writeStored('view', { sort: 'grade' });
    expect(items.view).toBe('{"sort":"grade"}');
    expect(readStored('view', null)).toEqual({ sort: 'grade' });
  });

  it('falls back when the key is missing, the value is not JSON or storage is unavailable', () => {
    stubStorage({ broken: '{' });
    expect(readStored('missing', 'auto')).toBe('auto');
    expect(readStored('broken', 'auto')).toBe('auto');
    vi.unstubAllGlobals();
    expect(readStored('missing', 'auto')).toBe('auto');
    expect(() => writeStored('missing', 'x')).not.toThrow();
  });
});
