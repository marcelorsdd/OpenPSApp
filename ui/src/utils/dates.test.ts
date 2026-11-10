import { describe, expect, it } from 'vitest';
import { dayKey, daysInMonth, monthKey, shiftMonth } from '@/utils/dates';

describe('date keys', () => {
  it('cut days and months in the given time zone', () => {
    expect(dayKey('2026-11-01T02:00:00Z', 'UTC')).toBe('2026-11-01');
    expect(dayKey('2026-11-01T02:00:00Z', 'America/Sao_Paulo')).toBe('2026-10-31');
    expect(monthKey('2026-11-01T02:00:00Z', 'America/Sao_Paulo')).toBe('2026-10');
    expect(monthKey(new Date('2026-11-09T12:00:00Z'), 'UTC')).toBe('2026-11');
  });

  it('shift months across years', () => {
    expect(shiftMonth('2026-01', -1)).toBe('2025-12');
    expect(shiftMonth('2026-12', 1)).toBe('2027-01');
    expect(shiftMonth('2026-11', 0)).toBe('2026-11');
  });

  it('count days in a month', () => {
    expect(daysInMonth('2026-02')).toBe(28);
    expect(daysInMonth('2028-02')).toBe(29);
    expect(daysInMonth('2026-10')).toBe(31);
  });
});
