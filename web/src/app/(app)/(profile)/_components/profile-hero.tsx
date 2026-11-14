import type { ReactNode } from 'react';
import { Avatar, AvatarFallback } from '@/components/ui/avatar';

export function ProfileHero({
  name,
  initial,
  summary,
  summaryShort,
  heroUrl,
  aside,
}: {
  name: string;
  initial: string;
  summary: string;
  summaryShort: string;
  heroUrl: string | null;
  aside: ReactNode;
}) {
  return (
    <section className="relative overflow-hidden">
      {heroUrl && <img src={heroUrl} alt="" className="absolute inset-0 size-full object-cover object-y-30 opacity-40 md:opacity-30" />}
      <div
        aria-hidden="true"
        className="absolute inset-0 bg-linear-to-b from-background/35 via-background/75 to-background max-md:from-background/20 max-md:via-background/60"
      />
      <div className="relative mx-auto flex max-w-page flex-wrap items-end justify-between gap-8 px-4 pt-28 pb-4 sm:px-6 md:pt-14 md:pb-10 lg:px-12">
        <div className="flex min-w-0 items-center gap-3.5 md:gap-6">
          <Avatar className="size-avatar-lg md:size-avatar-xl md:shadow-avatar">
            <AvatarFallback className="text-(length:--text-heading) font-extrabold md:text-(length:--text-display)">
              {initial}
            </AvatarFallback>
          </Avatar>
          <div className="min-w-0">
            <h1 className="truncate text-heading font-extrabold md:text-display">{name}</h1>
            <p className="mt-0.5 text-sm text-soft-foreground md:mt-2 md:text-lead">
              <span className="md:hidden">{summaryShort}</span>
              <span className="max-md:hidden">{summary}</span>
            </p>
          </div>
        </div>
        {aside}
      </div>
    </section>
  );
}
