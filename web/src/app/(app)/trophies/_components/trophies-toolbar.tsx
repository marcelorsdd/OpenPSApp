import { useTranslation } from 'react-i18next';
import { Select, SelectContent, SelectItem, SelectTrigger, SelectValue } from '@/components/ui/select';
import { Switch } from '@/components/ui/switch';
import type { SetSort } from '../_types/trophies';

export function TrophiesToolbar({
  sorts,
  sort,
  onSortChange,
  incompleteOnly,
  onIncompleteOnlyChange,
}: {
  sorts: SetSort[];
  sort: SetSort;
  onSortChange: (sort: SetSort) => void;
  incompleteOnly: boolean;
  onIncompleteOnlyChange: (value: boolean) => void;
}) {
  const { t } = useTranslation();
  const items = sorts.map((value) => ({ value, label: t(`trophies.sort.${value}`) }));
  return (
    <div className="flex flex-wrap items-center gap-3">
      <label className="flex items-center gap-2.5 text-sm text-soft-foreground">
        {t('trophies.incompleteOnly')}
        <Switch checked={incompleteOnly} onCheckedChange={onIncompleteOnlyChange} />
      </label>
      <Select items={items} value={sort} onValueChange={(value) => onSortChange(value as SetSort)}>
        <SelectTrigger aria-label={t('trophies.sort.label')} className="w-select max-md:w-auto">
          <span className="flex min-w-0 flex-1 gap-1">
            <span className="text-muted-foreground">{t('trophies.sortLabel')}</span>
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
  );
}
