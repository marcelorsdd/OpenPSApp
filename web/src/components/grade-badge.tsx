import type { Trophy } from '@openpsapp/contract/api';
import { useTranslation } from 'react-i18next';
import { Badge } from '@/components/ui/badge';

export function GradeBadge({
  grade,
  size = 'default',
  children,
}: {
  grade: Trophy['grade'];
  size?: 'default' | 'lg';
  children?: React.ReactNode;
}) {
  const { t } = useTranslation();
  return (
    <Badge variant={grade === 'unknown' ? 'outline' : grade} size={size}>
      {children ?? t(`grade.${grade}`)}
    </Badge>
  );
}
