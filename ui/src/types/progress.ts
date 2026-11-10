import type { EarnedTrophy, Grade } from '@/types/api';

export interface TopSet {
  setId: string;
  setName: string;
  titleId: string | null;
  count: number;
}

export interface MonthSummary {
  total: number;
  byGrade: Record<Grade, number>;
  topSet: TopSet | null;
  perWeek: number;
}

export interface DayGroup {
  day: string;
  items: EarnedTrophy[];
}
