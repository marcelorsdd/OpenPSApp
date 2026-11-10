import type { ReactNode } from 'react';

export default function Pill({ active, onClick, href: to, children }: { active: boolean; onClick?: () => void; href?: string; children: ReactNode }) {
  const style = `mx-1 px-5 py-3 rounded-full font-semibold border ${
    active ? 'bg-ink text-bg border-ink' : 'border-line text-muted hover:text-ink'
  }`;
  return to ? (
    <a href={to} className={style}>
      {children}
    </a>
  ) : (
    <button onClick={onClick} className={style}>
      {children}
    </button>
  );
}
