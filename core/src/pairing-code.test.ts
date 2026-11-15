import { describe, expect, it } from 'vitest';
import { displayUrl, formatUserCode, isUserCode, pairingLink, parseUserCode, sanitizeTypedCode } from './pairing-code';

describe('sanitizeTypedCode', () => {
  it('keeps the first eight uppercase letters and digits of what was typed', () => {
    expect(sanitizeTypedCode(' ab cd-23 45 ')).toBe('ABCD2345');
    expect(sanitizeTypedCode('ABCD2345XYZ')).toBe('ABCD2345');
    expect(sanitizeTypedCode('K7Q4.PX2A')).toBe('K7Q4PX2A');
  });
});

describe('parseUserCode', () => {
  it('accepts a code the way the server does, ignoring case, spaces and dashes', () => {
    expect(parseUserCode('ABCD2345')).toBe('ABCD2345');
    expect(parseUserCode(' abcd-2345 ')).toBe('ABCD2345');
    expect(parseUserCode('AB CD\t23-45')).toBe('ABCD2345');
  });

  it('rejects other separators, other lengths and characters outside the alphabet', () => {
    expect(parseUserCode('K7Q4.PX2A')).toBeNull();
    expect(parseUserCode('ABCD234')).toBeNull();
    expect(parseUserCode('ABCD23456')).toBeNull();
    expect(parseUserCode('ABCD2340')).toBeNull();
    expect(parseUserCode('')).toBeNull();
  });
});

describe('formatUserCode', () => {
  it('groups the code like the console shows it', () => {
    expect(formatUserCode('abcd2345')).toBe('ABCD-2345');
    expect(formatUserCode(' ab cd-23 45 ')).toBe('ABCD-2345');
    expect(formatUserCode('abc')).toBe('ABC');
    expect(formatUserCode('ABCD')).toBe('ABCD');
    expect(formatUserCode('ABCDE')).toBe('ABCD-E');
  });

  it('keeps at most eight characters', () => {
    expect(formatUserCode('ABCD2345XYZ')).toBe('ABCD-2345');
  });
});

describe('isUserCode', () => {
  it('accepts eight characters of the server alphabet only', () => {
    expect(isUserCode('ABCD2345')).toBe(true);
    expect(isUserCode('ABCD234')).toBe(false);
    expect(isUserCode('ABCD2340')).toBe(false);
    expect(isUserCode('ABCDI345')).toBe(false);
    expect(isUserCode('abcd2345')).toBe(false);
  });
});

describe('pairingLink', () => {
  it('puts the code without the dash in the fragment, which browsers never send to the server', () => {
    expect(pairingLink('https://openpsapp.app/link', 'ABCD-2345')).toBe('https://openpsapp.app/link#code=ABCD2345');
  });
});

describe('displayUrl', () => {
  it('drops the scheme', () => {
    expect(displayUrl('https://openpsapp.app/link')).toBe('openpsapp.app/link');
    expect(displayUrl('http://localhost:5174/link')).toBe('localhost:5174/link');
  });
});
