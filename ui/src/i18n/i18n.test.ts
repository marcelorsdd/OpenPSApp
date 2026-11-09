import { describe, expect, it } from 'vitest';
import i18n, { effectiveLanguage, LANGUAGES, parseChoice, resolveLanguage, resources } from '.';

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

describe('resolveLanguage', () => {
  it('maps console language tags to a supported locale', () => {
    expect(resolveLanguage('pt-BR')).toBe('pt-BR');
    expect(resolveLanguage('pt-PT')).toBe('pt-BR');
    expect(resolveLanguage('es-419')).toBe('es');
    expect(resolveLanguage('es-ES')).toBe('es');
    expect(resolveLanguage('en-GB')).toBe('en');
  });

  it('falls back to English', () => {
    expect(resolveLanguage('ja-JP')).toBe('en');
    expect(resolveLanguage(undefined)).toBe('en');
  });
});

describe('language choice', () => {
  it('accepts only known stored values', () => {
    expect(parseChoice('es')).toBe('es');
    expect(parseChoice('auto')).toBe('auto');
    expect(parseChoice('fr')).toBe('auto');
    expect(parseChoice(42)).toBe('auto');
  });

  it('follows the console in auto mode, then the browser', () => {
    expect(effectiveLanguage('auto', 'es-419', 'pt-BR')).toBe('es');
    expect(effectiveLanguage('auto', null, 'pt-BR')).toBe('pt-BR');
    expect(effectiveLanguage('auto', null, undefined)).toBe('en');
    expect(effectiveLanguage('en', 'pt-BR', 'pt-BR')).toBe('en');
  });
});
