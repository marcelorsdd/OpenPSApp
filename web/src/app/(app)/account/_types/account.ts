import type { LanguageChoice } from '@openpsapp/core';

export interface LanguageOption {
  value: LanguageChoice;
  label: string;
}

export type DeleteError = 'loginFailed' | 'failed';

export interface AccountTotals {
  games: number;
  trophies: number;
}
