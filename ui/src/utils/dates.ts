const formatters = new Map<string, Intl.DateTimeFormat>();

function formatter(timeZone?: string): Intl.DateTimeFormat {
  const key = timeZone ?? '';
  let f = formatters.get(key);
  if (!f) {
    f = new Intl.DateTimeFormat('en-US', { timeZone, year: 'numeric', month: '2-digit', day: '2-digit' });
    formatters.set(key, f);
  }
  return f;
}

export function dayKey(date: string | Date, timeZone?: string): string {
  const parts = formatter(timeZone).formatToParts(typeof date === 'string' ? new Date(date) : date);
  const part = (type: string) => parts.find((p) => p.type === type)?.value ?? '';
  return `${part('year')}-${part('month')}-${part('day')}`;
}

export function monthKey(date: string | Date, timeZone?: string): string {
  return dayKey(date, timeZone).slice(0, 7);
}

function splitMonth(month: string): [number, number] {
  const [y, m] = month.split('-').map(Number);
  return [y, m];
}

export function shiftMonth(month: string, delta: number): string {
  const [y, m] = splitMonth(month);
  const d = new Date(Date.UTC(y, m - 1 + delta, 1));
  return `${d.getUTCFullYear()}-${String(d.getUTCMonth() + 1).padStart(2, '0')}`;
}

export function daysInMonth(month: string): number {
  const [y, m] = splitMonth(month);
  return new Date(Date.UTC(y, m, 0)).getUTCDate();
}
