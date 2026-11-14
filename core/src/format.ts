export function formatDuration(seconds: number): string {
  if (seconds <= 0) return '0 min';
  if (seconds < 60) return '< 1 min';
  const minutes = Math.floor(seconds / 60);
  if (minutes < 60) return `${minutes} min`;
  const hours = Math.floor(minutes / 60);
  return `${hours} h ${String(minutes % 60).padStart(2, '0')} min`;
}

export function formatDate(iso: string | null, locale: string, timeZone?: string): string {
  if (!iso) return '—';
  return new Date(iso).toLocaleDateString(locale, { timeZone, day: '2-digit', month: '2-digit', year: 'numeric' });
}

export function formatDayMonth(iso: string | null, locale: string, timeZone?: string): string {
  if (!iso) return '—';
  return new Date(iso).toLocaleDateString(locale, { timeZone, day: '2-digit', month: '2-digit' });
}

export function formatDateTime(iso: string | null, locale: string, timeZone?: string): string {
  if (!iso) return '—';
  return new Date(iso).toLocaleString(locale, {
    timeZone,
    day: '2-digit',
    month: '2-digit',
    year: 'numeric',
    hour: '2-digit',
    minute: '2-digit',
  });
}

function utcNoon(day: string): Date {
  return new Date(`${day}T12:00:00Z`);
}

export function formatMonth(month: string, locale: string): string {
  const text = utcNoon(`${month}-15`).toLocaleDateString(locale, { timeZone: 'UTC', month: 'long', year: 'numeric' });
  return text.charAt(0).toUpperCase() + text.slice(1);
}

export function formatMonthName(month: string, locale: string): string {
  return utcNoon(`${month}-15`).toLocaleDateString(locale, { timeZone: 'UTC', month: 'long' });
}

export function formatLongDate(day: string, locale: string): string {
  return utcNoon(day).toLocaleDateString(locale, { timeZone: 'UTC', day: 'numeric', month: 'long', year: 'numeric' });
}

export function formatDayName(day: string, locale: string): string {
  return utcNoon(day).toLocaleDateString(locale, { timeZone: 'UTC', day: 'numeric', month: 'long' });
}

const RELATIVE_STEPS: [Intl.RelativeTimeFormatUnit, number][] = [
  ['minute', 60],
  ['hour', 24],
  ['day', 30],
  ['month', 12],
  ['year', Number.POSITIVE_INFINITY],
];

export function formatTimeAgo(iso: string, now: Date, locale: string): string {
  const format = new Intl.RelativeTimeFormat(locale, { numeric: 'auto' });
  let value = Math.floor((now.getTime() - new Date(iso).getTime()) / 60000);
  if (value < 1) return format.format(0, 'second');
  for (const [unit, size] of RELATIVE_STEPS) {
    if (value < size) return format.format(-value, unit);
    value = Math.floor(value / size);
  }
  return format.format(-value, 'year');
}

export function formatDecimal(value: number, locale: string): string {
  return new Intl.NumberFormat(locale, { maximumFractionDigits: 1 }).format(value);
}
