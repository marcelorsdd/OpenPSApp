import type { ReactNode } from 'react';

export default function Pill({
  active,
  onClick,
  href: to,
  children,
}: {
  active: boolean;
  onClick?: () => void;
  href?: string;
  children: ReactNode;
}) {
  const style = `mx-1 px-4 py-2 text-sm sm:px-5 sm:py-3 sm:text-base rounded-full font-semibold border ${
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
