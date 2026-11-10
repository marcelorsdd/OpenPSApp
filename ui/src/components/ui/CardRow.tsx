import type { ReactNode } from 'react';

export default function CardRow({
  to,
  icon,
  title,
  aside,
  children,
}: {
  to?: string;
  icon: ReactNode;
  title: ReactNode;
  aside?: ReactNode;
  children?: ReactNode;
}) {
  const body = (
    <>
      {icon}
      <div className="ml-5 flex-1 min-w-0">
        <div className="text-lg font-semibold truncate">{title}</div>
        {children}
      </div>
      {aside && <div className="ml-5 text-right flex-shrink-0">{aside}</div>}
    </>
  );
  const box = 'flex items-center p-4 rounded-2xl bg-surface border border-line';
  return (
    <li className="mb-3">
      {to ? (
        <a href={to} className={`${box} hover:bg-raised`}>
          {body}
        </a>
      ) : (
        <div className={box}>{body}</div>
      )}
    </li>
  );
}
