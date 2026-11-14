import { LANGUAGES } from '@openpsapp/core';
import { describe, expect, it } from 'vitest';
import i18n, { languageFor, readLanguageChoice, resources } from '.';

function keys(value: object, prefix = ''): string[] {
  return Object.entries(value).flatMap(([key, child]) =>
    typeof child === 'object' && child !== null ? keys(child as object, `${prefix}${key}.`) : [`${prefix}${key}`],
  );
}

describe('locales', () => {
  it('all translate the same keys', () => {
    const reference = keys(resources.en.translation).sort();
    for (const { code } of LANGUAGES) {
      expect(keys(resources[code].translation).sort()).toEqual(reference);
    }
  });

  it('pluralize counts', async () => {
    await i18n.changeLanguage('pt-BR');
    expect(i18n.t('profile.games', { count: 1 })).toBe('1 jogo');
    expect(i18n.t('profile.games', { count: 3 })).toBe('3 jogos');
    await i18n.changeLanguage('es');
    expect(i18n.t('profile.games', { count: 3 })).toBe('3 juegos');
    await i18n.changeLanguage('en');
    expect(i18n.t('profile.games', { count: 1 })).toBe('1 game');
  });
});

describe('language choice', () => {
  it('falls back to automatic without storage', () => {
    expect(readLanguageChoice()).toBe('auto');
  });

  it('uses the chosen language over the browser', () => {
    expect(languageFor('es')).toBe('es');
  });
});
