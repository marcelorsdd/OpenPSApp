import { cn } from 'cn';

export function LogoMark({ className }: { className?: string }) {
  return (
    <svg viewBox="-14 -14 128 128" aria-hidden="true" className={cn('size-8 shrink-0 rounded-lg bg-brand-tile', className)}>
      <circle
        cx="50"
        cy="50"
        r="42"
        fill="none"
        className="stroke-platinum"
        strokeWidth="6"
        strokeLinecap="round"
        strokeDasharray="205 264"
        transform="rotate(-4.75 50 50)"
      />
      <path d="M34 30 H66 V43 C66 54 59 61 50 61 C41 61 34 54 34 43 Z" className="fill-gold" />
      <path d="M34 34 H28 C28 42 31 47 36 49" fill="none" className="stroke-gold" strokeWidth="4" strokeLinecap="round" />
      <path d="M66 34 H72 C72 42 69 47 64 49" fill="none" className="stroke-gold" strokeWidth="4" strokeLinecap="round" />
      <rect x="47" y="60" width="6" height="8" className="fill-gold" />
      <rect x="39" y="67" width="22" height="6" rx="2" className="fill-gold" />
    </svg>
  );
}

export function Logo({ className, markClassName }: { className?: string; markClassName?: string }) {
  return (
    <span className={cn('flex items-center gap-2.5 text-lg font-bold text-foreground', className)}>
      <LogoMark className={markClassName} />
      OpenPSApp
    </span>
  );
}
