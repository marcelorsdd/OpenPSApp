import { describe, expect, it } from 'vitest';
import { effectiveLanguage, parseChoice, resolveLanguage } from './language';

describe('resolveLanguage', () => {
  it('maps language tags to a supported locale', () => {
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
