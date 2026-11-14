import { Spinner } from '@/components/ui/spinner';

export function PageLoader() {
  return (
    <div className="flex min-h-80 items-center justify-center text-muted-foreground">
      <Spinner className="size-6" />
    </div>
  );
}
