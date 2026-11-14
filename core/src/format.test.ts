import { describe, expect, it } from 'vitest';
import {
  formatDate,
  formatDateTime,
  formatDayMonth,
  formatDayName,
  formatDecimal,
  formatDuration,
  formatLongDate,
  formatMonth,
  formatMonthName,
  formatTimeAgo,
} from './format';

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

describe('formatDateTime', () => {
  it('shows the date with hours and minutes', () => {
    expect(formatDateTime('2026-11-14T16:05:00Z', 'pt-BR', 'UTC')).toMatch(/^14\/11\/2026,? 16:05$/);
    expect(formatDateTime(null, 'pt-BR')).toBe('—');
  });
});

describe('formatDayMonth', () => {
  it('shows day and month in the locale order', () => {
    expect(formatDayMonth('2026-11-08T04:35:53Z', 'pt-BR', 'America/Sao_Paulo')).toBe('08/11');
    expect(formatDayMonth('2026-11-08T04:35:53Z', 'en', 'America/Sao_Paulo')).toBe('11/08');
    expect(formatDayMonth(null, 'pt-BR')).toBe('—');
  });
});

describe('formatDayName and formatMonthName', () => {
  it('spell out a day without the year and a month alone', () => {
    expect(formatDayName('2026-11-08', 'pt-BR')).toBe('8 de novembro');
    expect(formatDayName('2026-11-08', 'en')).toBe('November 8');
    expect(formatMonthName('2026-11', 'pt-BR')).toBe('novembro');
    expect(formatMonthName('2026-11', 'es')).toBe('noviembre');
  });
});

describe('formatTimeAgo', () => {
  const now = new Date('2026-11-14T16:00:00Z');

  it('says now under a minute and counts minutes, hours and days after that', () => {
    expect(formatTimeAgo('2026-11-14T15:59:30Z', now, 'en')).toBe('now');
    expect(formatTimeAgo('2026-11-14T15:55:00Z', now, 'pt-BR')).toBe('há 5 minutos');
    expect(formatTimeAgo('2026-11-14T14:00:00Z', now, 'es')).toBe('hace 2 horas');
    expect(formatTimeAgo('2026-11-13T15:00:00Z', now, 'en')).toBe('yesterday');
    expect(formatTimeAgo('2026-09-10T15:00:00Z', now, 'en')).toBe('2 months ago');
  });
});

describe('formatDecimal', () => {
  it('keeps one decimal in the locale', () => {
    expect(formatDecimal(7.5, 'pt-BR')).toBe('7,5');
    expect(formatDecimal(2, 'en')).toBe('2');
  });
});
