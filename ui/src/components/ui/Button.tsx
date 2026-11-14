import type { ReactNode } from 'react';

const variants = {
  primary: 'bg-accent text-white hover:opacity-90',
  secondary: 'border border-line text-ink hover:bg-raised',
};

export default function Button({
  variant = 'primary',
  type = 'button',
  disabled,
  onClick,
  children,
}: {
  variant?: keyof typeof variants;
  type?: 'button' | 'submit';
  disabled?: boolean;
  onClick?: () => void;
  children: ReactNode;
}) {
  return (
    <button
      type={type}
      disabled={disabled}
      onClick={onClick}
      className={`px-6 py-3 rounded-xl font-semibold disabled:opacity-50 ${variants[variant]}`}
    >
      {children}
    </button>
  );
}
