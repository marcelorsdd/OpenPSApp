import { cn } from 'cn';
import { Gamepad2Icon } from 'lucide-react';

export function GameCover({ src, className }: { src: string | null; className?: string }) {
  if (!src) {
    return (
      <div className={cn('flex shrink-0 items-center justify-center bg-muted text-muted-foreground', className)}>
        <Gamepad2Icon className="size-1/2 max-h-8 max-w-8" />
      </div>
    );
  }
  return <img src={src} alt="" loading="lazy" decoding="async" className={cn('shrink-0 bg-muted object-cover', className)} />;
}
