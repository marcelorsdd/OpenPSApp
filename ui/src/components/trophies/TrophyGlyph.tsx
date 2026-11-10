import type { Grade } from '@/types/api';

const GRADE_TEXT: Record<Grade, string> = {
  platinum: 'text-platinum',
  gold: 'text-gold',
  silver: 'text-silver',
  bronze: 'text-bronze',
};

export default function TrophyGlyph({ grade, className = 'w-5 h-5' }: { grade: Grade | 'unknown'; className?: string }) {
  const color = grade === 'unknown' ? 'text-muted' : GRADE_TEXT[grade];
  return (
    <svg viewBox="0 0 24 24" className={`${className} ${color}`} fill="currentColor" aria-hidden="true">
      <path d="M7 3h10v5a5 5 0 0 1-10 0V3zm-4 1h3v2H5c0 2 1 3.3 2.4 3.8l-.6 1.9C4.5 10.9 3 8.8 3 6V4zm18 0v2c0 2.8-1.5 4.9-3.8 5.7l-.6-1.9C18 9.3 19 8 19 6h-1V4h3zM11 14h2v3h3v2H8v-2h3v-3z" />
    </svg>
  );
}
