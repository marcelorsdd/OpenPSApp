import { describe, expect, it } from 'vitest';
import { href, parseRoute } from '@/utils/router';

describe('parseRoute', () => {
  it('reads the console routes', () => {
    expect(parseRoute('')).toEqual({ name: 'profile' });
    expect(parseRoute('#/trophies')).toEqual({ name: 'trophies' });
    expect(parseRoute('#/game/PPSA01234')).toEqual({ name: 'game', titleId: 'PPSA01234' });
    expect(parseRoute('#/set/NPWR1_00')).toEqual({ name: 'set', setId: 'NPWR1_00' });
    expect(parseRoute('#/account')).toEqual({ name: 'account' });
  });

  it('decodes the ids it builds', () => {
    expect(parseRoute(href.game('A/B C'))).toEqual({ name: 'game', titleId: 'A/B C' });
    expect(parseRoute(href.set('NPWR1_00?x'))).toEqual({ name: 'set', setId: 'NPWR1_00?x' });
  });

  it('falls back to the profile on a malformed escape instead of throwing', () => {
    expect(parseRoute('#/game/%E0')).toEqual({ name: 'profile' });
    expect(parseRoute('#/set/%')).toEqual({ name: 'profile' });
  });

  it('falls back to the profile for unknown and removed routes', () => {
    expect(parseRoute('#/nope')).toEqual({ name: 'profile' });
    expect(parseRoute('#/link?code=ABCD2345')).toEqual({ name: 'profile' });
    expect(parseRoute('#/devices')).toEqual({ name: 'profile' });
  });
});
