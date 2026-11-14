import { describe, expect, it } from 'vitest';
import { linkCodeSchema } from './link-code.schema';

describe('linkCodeSchema', () => {
  it('accepts the eight characters of a pairing code, with or without the dash', () => {
    expect(linkCodeSchema.safeParse({ code: 'ABCD2345' }).success).toBe(true);
    expect(linkCodeSchema.safeParse({ code: 'abcd-2345' }).success).toBe(true);
  });

  it('rejects short codes and characters the server never issues', () => {
    const short = linkCodeSchema.safeParse({ code: 'ABCD234' });
    expect(short.success).toBe(false);
    expect(short.error?.issues[0].message).toBe('link.codeFormat');
    expect(linkCodeSchema.safeParse({ code: 'ABCD0145' }).success).toBe(false);
    expect(linkCodeSchema.safeParse({ code: 'ABCD.2345' }).success).toBe(false);
  });
});
