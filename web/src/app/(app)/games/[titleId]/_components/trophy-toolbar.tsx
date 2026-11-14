import type { TrophyFilter, TrophySort } from '@openpsapp/core';
import { useTranslation } from 'react-i18next';
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from '@/components/ui/select';
import { Switch } from '@/components/ui/switch';
import { Tabs, TabsList, TabsTrigger } from '@/components/ui/tabs';
import type { TrophyCounts } from '../_types/game';

export function TrophyToolbar({
  filters,
  filter,
  onFilterChange,
  counts,
  sorts,
  sort,
  onSortChange,
  showHidden,
  onShowHiddenChange,
}: {
  filters: TrophyFilter[];
  filter: TrophyFilter;
  onFilterChange: (filter: TrophyFilter) => void;
  counts: TrophyCounts;
  sorts: TrophySort[];
  sort: TrophySort;
  onSortChange: (sort: TrophySort) => void;
  showHidden: boolean;
  onShowHiddenChange: (value: boolean) => void;
}) {
  const { t } = useTranslation();
  const count: Record<TrophyFilter, number> = { all: counts.all, earned: counts.earned, locked: counts.locked };
  const items = sorts.map((value) => ({ value, label: t(`game.sort.${value}`) }));
  return (
    <div className="flex flex-wrap items-center justify-between gap-3">
      <Tabs value={filter} onValueChange={(value) => onFilterChange(value as TrophyFilter)} className="max-md:w-full">
        <TabsList
          aria-label={t('game.filter.label')}
          className="max-md:grid max-md:w-full max-md:grid-cols-3 max-md:rounded-xl max-md:border max-md:border-border max-md:bg-card max-md:p-1"
        >
          {filters.map((value) => (
            <TabsTrigger key={value} value={value} className="max-md:py-2 max-md:font-semibold">
              {t(`game.filter.${value}`)}
              <span className="text-muted-foreground group-data-active:text-inherit max-md:hidden">{count[value]}</span>
            </TabsTrigger>
          ))}
        </TabsList>
      </Tabs>
      <div className="flex flex-wrap items-center gap-4 max-md:w-full max-md:justify-between">
        {counts.hidden > 0 && (
          <label className="flex items-center gap-2.5 text-sm text-soft-foreground">
            {t('game.showHidden', { count: counts.hidden })}
            <Switch checked={showHidden} onCheckedChange={onShowHiddenChange} />
          </label>
        )}
        <Select items={items} value={sort} onValueChange={(value) => onSortChange(value as TrophySort)}>
          <SelectTrigger aria-label={t('game.sort.label')} className="w-select max-md:w-auto">
            <span className="flex min-w-0 flex-1 gap-1">
              <span className="text-muted-foreground">{t('game.sortLabel')}</span>
              <SelectValue />
            </span>
          </SelectTrigger>
          <SelectContent>
            {items.map((item) => (
              <SelectItem key={item.value} value={item.value}>
                {item.label}
              </SelectItem>
            ))}
          </SelectContent>
        </Select>
      </div>
    </div>
  );
}
