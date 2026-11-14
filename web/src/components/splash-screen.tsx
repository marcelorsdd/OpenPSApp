import { LogoMark } from '@/components/brand/logo';
import { Spinner } from '@/components/ui/spinner';

export function SplashScreen() {
  return (
    <div className="flex min-h-dvh flex-col items-center justify-center gap-6 text-muted-foreground">
      <LogoMark className="size-14" />
      <Spinner className="size-5" />
    </div>
  );
}
