export interface TrophyView {
  masked: boolean;
  name: string;
  description: string;
  iconUrl: string | null;
}

export type TrophySort = 'default' | 'earned' | 'grade';

export type TrophyFilter = 'all' | 'earned' | 'locked';
