import type { ReactNode } from 'react';

export default function Panel({ children }: { children: ReactNode }) {
  return <div className="mt-8 rounded-2xl bg-surface border border-line p-8">{children}</div>;
}
