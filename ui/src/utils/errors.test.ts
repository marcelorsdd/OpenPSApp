import { LANGUAGES } from '@openpsapp/core';
import { describe, expect, it } from 'vitest';
import i18n from '@/i18n';
import { ERROR_CODES, errorMessageKey } from '@/utils/errors';

describe('errorMessageKey', () => {
  it('maps the codes the console sends to their own message', () => {
    expect(errorMessageKey('no_profile')).toBe('errors.no_profile');
    expect(errorMessageKey('cloud_unavailable')).toBe('errors.cloud_unavailable');
    expect(errorMessageKey('forbidden')).toBe('errors.forbidden');
  });

  it('falls back to the generic message for unknown codes and failures without one', () => {
    expect(errorMessageKey('something_new')).toBe('errors.generic');
    expect(errorMessageKey('')).toBe('errors.generic');
    expect(errorMessageKey(null)).toBe('errors.generic');
  });

  it('has a translated message for every code in every language', async () => {
    for (const { code: language } of LANGUAGES) {
      await i18n.changeLanguage(language);
      for (const code of [...ERROR_CODES, 'generic']) {
        const key = errorMessageKey(code === 'generic' ? null : code);
        expect(i18n.exists(key), `${language} ${key}`).toBe(true);
        expect(i18n.t(key)).not.toBe(key);
      }
    }
  });

  it('reads naturally in each language', async () => {
    await i18n.changeLanguage('en');
    expect(i18n.t(errorMessageKey('busy'))).toBe('The console is busy. Try again in a moment.');
    await i18n.changeLanguage('pt-BR');
    expect(i18n.t(errorMessageKey('already_linked'))).toBe('Este perfil já está conectado a uma conta.');
    await i18n.changeLanguage('es');
    expect(i18n.t(errorMessageKey('unknown'))).toBe('Algo salió mal. Inténtalo de nuevo.');
  });
});
