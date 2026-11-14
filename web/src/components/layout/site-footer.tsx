import type { ReactNode } from 'react';

export function SiteFooter({ start, end }: { start: ReactNode; end: ReactNode }) {
  return (
    <footer className="border-t border-border-subtle px-4 py-5 text-sm text-muted-foreground sm:px-6 lg:px-12">
      <div className="flex flex-wrap items-center justify-between gap-3">
        <span className="min-w-0">{start}</span>
        <span className="flex gap-5">{end}</span>
      </div>
    </footer>
  );
}
