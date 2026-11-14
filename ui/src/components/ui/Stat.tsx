import type { ReactNode } from 'react';

export default function Stat({ label, children }: { label: string; children: ReactNode }) {
  return (
    <div className="rounded-2xl bg-surface border border-line p-4 sm:p-6">
      <div className="text-muted text-sm uppercase tracking-wide">{label}</div>
      <div className="mt-2 text-xl sm:text-2xl font-semibold whitespace-nowrap">{children}</div>
    </div>
  );
}
