import { useState } from 'react';
import { useTranslation } from 'react-i18next';
import EarnedTrophyRow from '@/components/trophies/EarnedTrophyRow';
import ErrorBox from '@/components/ui/ErrorBox';
import Loading from '@/components/ui/Loading';
import { useApi } from '@/hooks/useApi';
import { endpoints } from '@/services/api';
import { formatLongDate } from '@/utils/format';
import { groupByDay } from '@/utils/progress';

const PAGE = 50;

export default function TrophyTimeline() {
  const { t, i18n } = useTranslation();
  const earned = useApi(endpoints.earnedTrophies());
  const [shown, setShown] = useState(PAGE);

  if (earned.error) return <ErrorBox message={earned.error} onRetry={earned.reload} />;
  if (!earned.data) return <Loading />;
  if (earned.data.length === 0) return <p className="mt-8 text-muted">{t('trophies.timelineEmpty')}</p>;

  return (
    <>
      {groupByDay(earned.data.slice(0, shown)).map((group) => (
        <section key={group.day} className="mt-8">
          <h2 className="text-lg font-semibold mb-3">{formatLongDate(group.day, i18n.language)}</h2>
          <ul>
            {group.items.map((item) => (
              <EarnedTrophyRow key={`${item.setId}:${item.trophy.id}`} item={item} showDate={false} />
            ))}
          </ul>
        </section>
      ))}
      {shown < earned.data.length && (
        <button className="mt-4 px-6 py-3 rounded-xl border border-line font-semibold hover:bg-raised" onClick={() => setShown(shown + PAGE)}>
          {t('trophies.loadMore')}
        </button>
      )}
    </>
  );
}
