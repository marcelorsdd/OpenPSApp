export type SyncGrade = 'platinum' | 'gold' | 'silver' | 'bronze' | 'unknown';

export interface SyncProfile {
  name: string;
  language: string | null;
}

export interface SyncGame {
  titleId: string;
  name: string;
  playtimeSeconds: number;
  lastPlayedAt: string | null;
  iconId: string | null;
  heroId: string | null;
  trophySetId: string | null;
  hidden: boolean;
}

export interface SyncSetRef {
  id: string;
  version: string;
}

export interface SyncManifest {
  profile: SyncProfile;
  games: SyncGame[];
  sets: SyncSetRef[];
  assets: string[];
}

export interface SyncManifestResponse {
  needSets: string[];
  needAssets: string[];
}

export interface SyncTrophy {
  id: number;
  name: string;
  description: string;
  grade: SyncGrade;
  hidden: boolean;
  earned: boolean;
  earnedAt: string | null;
  iconId: string | null;
}

export interface SyncTrophySet {
  id: string;
  version: string;
  name: string;
  iconId: string | null;
  trophies: SyncTrophy[];
}
