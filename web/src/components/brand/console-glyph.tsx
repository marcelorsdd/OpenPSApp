import { cn } from 'cn';

export function ConsoleGlyph({ className }: { className?: string }) {
  return (
    <svg viewBox="0 0 24 24" aria-hidden="true" className={cn('size-7.5', className)}>
      <rect x="7" y="2.5" width="10" height="19" rx="3" fill="none" stroke="currentColor" strokeWidth="1.6" />
      <path d="M12 2.5v19" stroke="currentColor" strokeWidth="1.2" />
    </svg>
  );
}
