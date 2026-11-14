import type { EarnedTrophy, Grade, GradeCount } from '@openpsapp/contract/api';

export type SetSort = 'recent' | 'progress' | 'name';

export interface SetCardView {
  id: string;
  name: string;
  titleId: string | null;
  iconUrl: string | null;
  heroUrl: string | null;
  progress: number;
  earned: number;
  total: number;
  lastEarnedAt: string | null;
  counts: Record<Grade, GradeCount>;
}

export interface GradeTotal {
  grade: Grade;
  earned: number;
  total: number;
}

export interface TimelineDay {
  day: string;
  count: number;
  items: EarnedTrophy[];
}
