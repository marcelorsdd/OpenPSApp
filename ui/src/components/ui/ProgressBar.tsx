export default function ProgressBar({ value, large = false }: { value: number; large?: boolean }) {
  const pct = Math.max(0, Math.min(100, value));
  return (
    <div className="flex items-center">
      <div className="flex-1 h-2 rounded-full bg-line overflow-hidden" role="progressbar" aria-valuenow={pct} aria-valuemin={0} aria-valuemax={100}>
        <div className="h-full rounded-full bg-accent" style={{ width: `${pct}%` }} />
      </div>
      <span className={`ml-3 tabular-nums ${large ? 'text-xl font-semibold' : 'text-sm text-muted'}`}>{pct}%</span>
    </div>
  );
}
