import { parseUserCode } from '@openpsapp/core';
import { z } from 'zod';

export const linkCodeSchema = z.object({
  code: z.string().refine((value) => parseUserCode(value) !== null, { message: 'link.codeFormat' }),
});

export type LinkCodeValues = z.infer<typeof linkCodeSchema>;
