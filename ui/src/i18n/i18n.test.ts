import { LANGUAGES } from '@openpsapp/core';
import { describe, expect, it } from 'vitest';
import i18n, { resources } from '.';

function keys(obj: object, prefix = ''): string[] {
  return Object.entries(obj).flatMap(([k, v]) =>
    typeof v === 'object' && v !== null ? keys(v as object, `${prefix}${k}.`) : [`${prefix}${k}`],
  );
}

describe('locales', () => {
  it('all translate the same keys', () => {
    const reference = keys(resources['pt-BR'].translation).sort();
    for (const { code } of LANGUAGES) {
      expect(keys(resources[code].translation).sort()).toEqual(reference);
    }
  });

  it('pluralize counts', async () => {
    await i18n.changeLanguage('pt-BR');
    expect(i18n.t('profile.games', { count: 1 })).toBe('1 jogo');
    expect(i18n.t('profile.games', { count: 3 })).toBe('3 jogos');
    await i18n.changeLanguage('en');
    expect(i18n.t('profile.games', { count: 1 })).toBe('1 game');
  });
});
