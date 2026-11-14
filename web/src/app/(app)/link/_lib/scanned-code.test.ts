import { describe, expect, it } from 'vitest';
import { readScannedCode } from './scanned-code';

const origin = 'https://openpsapp.app';

describe('readScannedCode', () => {
  it('reads the link page URL the console shows', () => {
    expect(readScannedCode('https://openpsapp.app/link?code=ABCD2345', origin)).toBe('ABCD2345');
    expect(readScannedCode('https://openpsapp.app/link/?code=abcd-2345', origin)).toBe('ABCD2345');
    expect(readScannedCode('  https://openpsapp.app/link?code=ABCD2345\n', origin)).toBe('ABCD2345');
  });

  it('accepts a bare pairing code with or without the dash', () => {
    expect(readScannedCode('ABCD2345', origin)).toBe('ABCD2345');
    expect(readScannedCode('ABCD-2345', origin)).toBe('ABCD2345');
  });

  it('rejects URLs of other sites, paths or schemes', () => {
    expect(readScannedCode('https://evil.example/link?code=ABCD2345', origin)).toBeNull();
    expect(readScannedCode('http://openpsapp.app/link?code=ABCD2345', origin)).toBeNull();
    expect(readScannedCode('https://openpsapp.app.evil.example/link?code=ABCD2345', origin)).toBeNull();
    expect(readScannedCode('https://openpsapp.app@evil.example/link?code=ABCD2345', origin)).toBeNull();
    expect(readScannedCode('https://user@openpsapp.app/link?code=ABCD2345', origin)).toBeNull();
    expect(readScannedCode('https://openpsapp.app/account?code=ABCD2345', origin)).toBeNull();
    expect(readScannedCode('https://openpsapp.app/link/extra?code=ABCD2345', origin)).toBeNull();
    expect(readScannedCode('javascript:alert(1)//ABCD2345', origin)).toBeNull();
  });

  it('rejects links without exactly one valid code', () => {
    expect(readScannedCode('https://openpsapp.app/link', origin)).toBeNull();
    expect(readScannedCode('https://openpsapp.app/link?code=ABCD', origin)).toBeNull();
    expect(readScannedCode('https://openpsapp.app/link?code=ABCD2345&code=WXYZ2345', origin)).toBeNull();
    expect(readScannedCode('https://openpsapp.app/link?code=ABCD0OI1', origin)).toBeNull();
  });

  it('rejects anything else', () => {
    expect(readScannedCode('', origin)).toBeNull();
    expect(readScannedCode('hello world', origin)).toBeNull();
    expect(readScannedCode('ABCD23456', origin)).toBeNull();
    expect(readScannedCode('ABCD_2345', origin)).toBeNull();
    expect(readScannedCode('WIFI:S:home;T:WPA;P:secret;;', origin)).toBeNull();
  });
});
