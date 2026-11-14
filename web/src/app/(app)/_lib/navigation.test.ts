import { describe, expect, it } from 'vitest';
import { isFocusedRoute, sectionFor, shortcutTarget } from './navigation';

describe('sectionFor', () => {
  it('marks the tab each page belongs to', () => {
    expect(sectionFor('/')).toBe('profile');
    expect(sectionFor('/trophies')).toBe('trophies');
    expect(sectionFor('/games/PPSA01234')).toBe('trophies');
    expect(sectionFor('/account')).toBe('account');
    expect(sectionFor('/link')).toBe('account');
    expect(sectionFor('/nope')).toBeNull();
  });
});

describe('isFocusedRoute', () => {
  it('is true when any matched route asks for the focused layout', () => {
    expect(isFocusedRoute([{ handle: undefined }, { handle: { focused: true } }])).toBe(true);
    expect(isFocusedRoute([{ handle: undefined }, { handle: {} }])).toBe(false);
  });
});

describe('shortcutTarget', () => {
  const keys = { profile: 'p', trophies: 't', account: 'c' };

  it('maps the second key of a g-sequence to a page', () => {
    expect(shortcutTarget('t', keys)).toBe('/trophies');
    expect(shortcutTarget('C', keys)).toBe('/account');
    expect(shortcutTarget('x', keys)).toBeNull();
  });
});
