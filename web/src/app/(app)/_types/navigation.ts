export type Section = 'profile' | 'trophies' | 'account';

export interface NavItem {
  section: Section;
  to: string;
}

export type ShortcutKeys = Record<Section, string>;
