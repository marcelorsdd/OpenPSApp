import type { ReactNode } from 'react';

export default function Stat({ label, children }: { label: string; children: ReactNode }) {
  return (
    <div className="rounded-2xl bg-surface border border-line p-6">
      <div className="text-muted text-sm uppercase tracking-wide">{label}</div>
      <div className="mt-2 text-2xl font-semibold">{children}</div>
    </div>
  );
}
