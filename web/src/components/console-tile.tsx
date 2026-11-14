import { cn } from 'cn';
import { ConsoleGlyph } from '@/components/brand/console-glyph';

export function ConsoleTile({ size = 'default', className }: { size?: 'default' | 'sm'; className?: string }) {
  return (
    <div
      className={cn(
        'flex shrink-0 items-center justify-center rounded-xl bg-brand-tile text-foreground',
        size === 'sm' ? 'size-12' : 'size-14',
        className,
      )}
    >
      <ConsoleGlyph className={size === 'sm' ? 'size-6.5' : undefined} />
    </div>
  );
}
