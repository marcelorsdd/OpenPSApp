import type { ReactNode } from 'react';
import { useTranslation } from 'react-i18next';
import { GRADES, type Grade, type GradeCount, type Profile } from '../api';
import { LANGUAGES, type LanguageChoice } from '../i18n';
import { href } from '../router';

const GRADE_TEXT: Record<Grade, string> = {
  platinum: 'text-platinum',
  gold: 'text-gold',
  silver: 'text-silver',
  bronze: 'text-bronze',
};

export function TrophyGlyph({ grade, className = 'w-5 h-5' }: { grade: Grade | 'unknown'; className?: string }) {
  const color = grade === 'unknown' ? 'text-muted' : GRADE_TEXT[grade];
  return (
    <svg viewBox="0 0 24 24" className={`${className} ${color}`} fill="currentColor" aria-hidden="true">
      <path d="M7 3h10v5a5 5 0 0 1-10 0V3zm-4 1h3v2H5c0 2 1 3.3 2.4 3.8l-.6 1.9C4.5 10.9 3 8.8 3 6V4zm18 0v2c0 2.8-1.5 4.9-3.8 5.7l-.6-1.9C18 9.3 19 8 19 6h-1V4h3zM11 14h2v3h3v2H8v-2h3v-3z" />
    </svg>
  );
}

export function GradeCounts({ counts }: { counts: Record<Grade, GradeCount | number> }) {
  const { t } = useTranslation();
  return (
    <div className="flex flex-wrap -mx-2">
      {GRADES.map((g) => {
        const c = counts[g];
        return (
          <span key={g} className="flex items-center mx-2" title={t(`grade.${g}`)}>
            <TrophyGlyph grade={g} />
            <span className="ml-1 tabular-nums">{typeof c === 'number' ? c : `${c.earned}/${c.total}`}</span>
          </span>
        );
      })}
    </div>
  );
}

export function ProgressBar({ value, large = false }: { value: number; large?: boolean }) {
  const pct = Math.max(0, Math.min(100, value));
  return (
    <div className="flex items-center">
      <div className="flex-1 h-2 rounded-full bg-line overflow-hidden" role="progressbar" aria-valuenow={pct} aria-valuemin={0} aria-valuemax={100}>
        <div className="h-full rounded-full bg-accent" style={{ width: `${pct}%` }} />
      </div>
      <span className={`ml-3 tabular-nums ${large ? 'text-xl font-semibold' : 'text-sm text-muted'}`}>{pct}%</span>
    </div>
  );
}

export function Cover({ src, size }: { src: string | null; size: string }) {
  if (!src) return <div className={`${size} rounded-xl bg-raised flex-shrink-0`} aria-hidden="true" />;
  return <img src={src} alt="" className={`${size} rounded-xl object-cover flex-shrink-0 bg-raised`} loading="lazy" />;
}

export function CardRow({
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

export function Pill({ active, onClick, href: to, children }: { active: boolean; onClick?: () => void; href?: string; children: ReactNode }) {
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

export function Section({ title, children }: { title: string; children: ReactNode }) {
  return (
    <section className="mt-10">
      <h2 className="text-xl font-semibold mb-4">{title}</h2>
      {children}
    </section>
  );
}

export function Stat({ label, children }: { label: string; children: ReactNode }) {
  return (
    <div className="rounded-2xl bg-surface border border-line p-6">
      <div className="text-muted text-sm uppercase tracking-wide">{label}</div>
      <div className="mt-2 text-2xl font-semibold">{children}</div>
    </div>
  );
}

export function TrophyTotals({ label, trophies }: { label: string; trophies: Profile['totals']['trophies'] }) {
  const { t } = useTranslation();
  return (
    <Stat label={label}>
      <div>
        {trophies.earned}
        <span className="text-muted text-lg font-normal"> {t('common.outOf', { total: trophies.total })}</span>
      </div>
      <div className="mt-3 text-base font-normal">
        <GradeCounts counts={trophies} />
      </div>
    </Stat>
  );
}

export function BackLink() {
  const { t } = useTranslation();
  const back = (e: { preventDefault: () => void }) => {
    if (window.history.length > 1) {
      e.preventDefault();
      window.history.back();
    }
  };
  return (
    <a href={href.profile()} onClick={back} className="inline-block text-muted hover:text-ink mb-6">
      {t('common.back')}
    </a>
  );
}

export function Loading() {
  const { t } = useTranslation();
  return <div className="py-24 text-center text-muted">{t('common.loading')}</div>;
}

export function ErrorBox({ message, onRetry }: { message: string; onRetry: () => void }) {
  const { t } = useTranslation();
  return (
    <div className="py-24 text-center">
      <p className="text-muted">{t('common.loadError', { message })}</p>
      <button className="mt-6 px-6 py-3 rounded-xl bg-accent font-semibold" onClick={onRetry}>
        {t('common.retry')}
      </button>
    </div>
  );
}

export function LanguagePicker({ value, onChange }: { value: LanguageChoice; onChange: (value: LanguageChoice) => void }) {
  const { t } = useTranslation();
  return (
    <select
      aria-label={t('language.label')}
      value={value}
      onChange={(e) => onChange(e.target.value as LanguageChoice)}
      className="ml-3 px-4 py-3 rounded-full bg-surface border border-line text-ink font-semibold"
    >
      <option value="auto">{t('language.auto')}</option>
      {LANGUAGES.map((l) => (
        <option key={l.code} value={l.code}>
          {l.label}
        </option>
      ))}
    </select>
  );
}
