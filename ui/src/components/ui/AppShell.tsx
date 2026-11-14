import type { ReactNode } from 'react';
import { href } from '@/utils/router';

export default function AppShell({ nav, children }: { nav: ReactNode; children: ReactNode }) {
  return (
    <div className="max-w-5xl mx-auto px-4 sm:px-8 pb-16">
      <header className="flex flex-wrap items-center justify-between py-4 sm:py-8">
        <a href={href.profile()} className="flex items-center mr-4 my-2 text-xl sm:text-2xl font-bold tracking-tight">
          <img src="/favicon.svg" alt="" className="w-9 h-9 sm:w-10 sm:h-10" />
          <span className="ml-3">OpenPSApp</span>
        </a>
        <nav className="flex flex-wrap items-center -mx-1 my-2">{nav}</nav>
      </header>
      {children}
    </div>
  );
}
