import { describe, expect, it } from 'vitest';
import { ApiError } from './api';
import { shouldRetry } from './query-client';

describe('shouldRetry', () => {
  it('never retries client errors', () => {
    expect(shouldRetry(0, new ApiError(404, 'not_found'))).toBe(false);
    expect(shouldRetry(0, new ApiError(401, 'unauthorized'))).toBe(false);
  });

  it('retries server and network errors twice', () => {
    expect(shouldRetry(0, new ApiError(502, null))).toBe(true);
    expect(shouldRetry(1, new TypeError('Failed to fetch'))).toBe(true);
    expect(shouldRetry(2, new TypeError('Failed to fetch'))).toBe(false);
  });
});
