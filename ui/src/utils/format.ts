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

function utcNoon(day: string): Date {
  return new Date(`${day}T12:00:00Z`);
}

export function formatMonth(month: string, locale: string): string {
  const text = utcNoon(`${month}-15`).toLocaleDateString(locale, { timeZone: 'UTC', month: 'long', year: 'numeric' });
  return text.charAt(0).toUpperCase() + text.slice(1);
}

export function formatLongDate(day: string, locale: string): string {
  return utcNoon(day).toLocaleDateString(locale, { timeZone: 'UTC', day: 'numeric', month: 'long', year: 'numeric' });
}
