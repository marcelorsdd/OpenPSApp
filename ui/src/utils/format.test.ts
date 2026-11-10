import { describe, expect, it } from 'vitest';
import { formatDate, formatDuration, formatLongDate, formatMonth } from '@/utils/format';

describe('formatDuration', () => {
  it('shows hours and minutes like the console', () => {
    expect(formatDuration(51494)).toBe('14 h 18 min');
    expect(formatDuration(72283)).toBe('20 h 04 min');
    expect(formatDuration(3600)).toBe('1 h 00 min');
  });

  it('shows minutes below an hour', () => {
    expect(formatDuration(73)).toBe('1 min');
    expect(formatDuration(59 * 60)).toBe('59 min');
  });

  it('handles nothing played', () => {
    expect(formatDuration(0)).toBe('0 min');
    expect(formatDuration(30)).toBe('< 1 min');
  });
});

describe('formatDate', () => {
  it('formats in the given locale and time zone', () => {
    expect(formatDate('2026-11-08T04:35:53Z', 'pt-BR', 'America/Sao_Paulo')).toBe('08/11/2026');
    expect(formatDate('2026-11-08T01:35:53Z', 'pt-BR', 'America/Sao_Paulo')).toBe('07/11/2026');
    expect(formatDate('2026-11-08T04:35:53Z', 'en', 'America/Sao_Paulo')).toBe('11/08/2026');
  });

  it('returns a dash for missing dates', () => {
    expect(formatDate(null, 'pt-BR')).toBe('—');
  });
});

describe('month and day labels', () => {
  it('spell out the month and capitalize it', () => {
    expect(formatMonth('2026-11', 'pt-BR')).toBe('Novembro de 2026');
    expect(formatMonth('2026-11', 'en')).toBe('November 2026');
  });

  it('spell out a day', () => {
    expect(formatLongDate('2026-11-08', 'pt-BR')).toBe('8 de novembro de 2026');
    expect(formatLongDate('2026-11-08', 'en')).toBe('November 8, 2026');
  });
});
