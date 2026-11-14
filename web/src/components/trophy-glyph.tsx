import type { Trophy } from '@openpsapp/contract/api';
import { cn } from 'cn';

const GRADE_COLOR: Record<Trophy['grade'], string> = {
  platinum: 'text-platinum',
  gold: 'text-gold',
  silver: 'text-silver',
  bronze: 'text-bronze',
  unknown: 'text-muted-foreground',
};

export function TrophyGlyph({ grade, label, className }: { grade: Trophy['grade']; label?: string; className?: string }) {
  return (
    <svg
      viewBox="0 0 24 24"
      role={label ? 'img' : undefined}
      aria-label={label}
      aria-hidden={label ? undefined : true}
      className={cn('size-4.5 shrink-0', GRADE_COLOR[grade], className)}
    >
      <path d="M7 3h10v5a5 5 0 0 1-10 0z" fill="currentColor" />
      <path
        d="M7 5H4c0 3 1.5 4.5 3.5 5M17 5h3c0 3-1.5 4.5-3.5 5"
        fill="none"
        stroke="currentColor"
        strokeWidth="1.8"
        strokeLinecap="round"
      />
      <rect x="11" y="12.5" width="2" height="4" fill="currentColor" />
      <rect x="7.5" y="16.5" width="9" height="3" rx="1" fill="currentColor" />
    </svg>
  );
}
