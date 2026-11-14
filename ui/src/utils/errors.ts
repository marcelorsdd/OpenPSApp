export const ERROR_CODES = [
  'no_profile',
  'already_linked',
  'cloud_unavailable',
  'console_only',
  'not_configured',
  'forbidden',
  'busy',
  'internal',
  'not_found',
  'method_not_allowed',
  'bad_request',
  'headers_too_large',
  'too_large',
  'save_failed',
  'playtime_unavailable',
  'path_not_allowed',
  'appdb_copy_failed',
] as const;

const KNOWN: readonly string[] = ERROR_CODES;

export function errorMessageKey(code: string | null): string {
  return code !== null && KNOWN.includes(code) ? `errors.${code}` : 'errors.generic';
}
