import { useState, type ReactNode } from 'react';
import { useTranslation } from 'react-i18next';
import GradeCounts from '@/components/trophies/GradeCounts';
import Cover from '@/components/ui/Cover';
import Section from '@/components/ui/Section';
import type { EarnedTrophy, TrophySet } from '@/types/api';
import { monthKey, shiftMonth } from '@/utils/dates';
import { formatMonth } from '@/utils/format';
import { monthSummary } from '@/utils/progress';
import { href } from '@/utils/router';

export default function MonthCard({ earned, sets }: { earned: EarnedTrophy[]; sets: TrophySet[] }) {
  const { t, i18n } = useTranslation();
  const now = new Date();
  const current = monthKey(now);
  const oldest = earned[earned.length - 1]?.trophy.earnedAt;
  const first = oldest ? monthKey(oldest) : current;
  const [month, setMonth] = useState(current);
  const summary = monthSummary(earned, month, now);
  const top = summary.topSet;
  const topIcon = top ? (sets.find((s) => s.id === top.setId)?.iconUrl ?? null) : null;
  const pace = new Intl.NumberFormat(i18n.language, { maximumFractionDigits: 1 }).format(summary.perWeek);
  const arrow = 'w-12 h-12 rounded-full border border-line text-xl hover:bg-raised disabled:opacity-30';

  return (
    <Section title={t('month.title')}>
      <div className="rounded-2xl bg-surface border border-line p-6">
        <div className="flex items-center justify-between">
          <button className={arrow} disabled={month <= first} onClick={() => setMonth(shiftMonth(month, -1))} aria-label={t('month.previous')}>
            ‹
          </button>
          <div className="text-xl font-semibold">{formatMonth(month, i18n.language)}</div>
          <button className={arrow} disabled={month >= current} onClick={() => setMonth(shiftMonth(month, 1))} aria-label={t('month.next')}>
            ›
          </button>
        </div>
        {summary.total === 0 || !top ? (
          <p className="mt-6 text-center text-muted">{t('month.empty')}</p>
        ) : (
          <div className="mt-6 grid sm:grid-cols-3 gap-6">
            <Block label={t('profile.trophies')}>
              <div className="text-2xl font-semibold">{summary.total}</div>
              <div className="mt-2">
                <GradeCounts counts={summary.byGrade} />
              </div>
            </Block>
            <Block label={t('month.topGame')}>
              <a href={top.titleId ? href.game(top.titleId) : href.set(top.setId)} className="flex items-center hover:text-ink">
                <Cover src={topIcon} size="w-14 h-14" />
                <div className="ml-3 min-w-0">
                  <div className="font-semibold truncate">{top.setName}</div>
                  <div className="text-sm text-muted">{t('month.trophiesInSet', { n: top.count })}</div>
                </div>
              </a>
            </Block>
            <Block label={t('month.pace')}>
              <div className="text-2xl font-semibold">{t('month.perWeek', { value: pace })}</div>
            </Block>
          </div>
        )}
      </div>
    </Section>
  );
}

function Block({ label, children }: { label: string; children: ReactNode }) {
  return (
    <div>
      <div className="text-sm uppercase tracking-wide text-muted mb-2">{label}</div>
      {children}
    </div>
  );
}
