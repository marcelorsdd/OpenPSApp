export const USER_CODE_LENGTH = 8;
export const USER_CODE_ALPHABET = 'ABCDEFGHJKLMNPQRSTUVWXYZ23456789';

export function sanitizeTypedCode(raw: string): string {
  return raw
    .toUpperCase()
    .replace(/[^A-Z0-9]/g, '')
    .slice(0, USER_CODE_LENGTH);
}

export function formatUserCode(raw: string): string {
  const code = sanitizeTypedCode(raw);
  return code.length > 4 ? `${code.slice(0, 4)}-${code.slice(4)}` : code;
}

export function isUserCode(code: string): boolean {
  return code.length === USER_CODE_LENGTH && [...code].every((c) => USER_CODE_ALPHABET.includes(c));
}

export function parseUserCode(input: string): string | null {
  const code = input.toUpperCase().replace(/[\s-]/g, '');
  return isUserCode(code) ? code : null;
}

export function pairingLink(verificationUri: string, userCode: string): string {
  return `${verificationUri}?code=${userCode.replace(/-/g, '')}`;
}

export function displayUrl(uri: string): string {
  return uri.replace(/^https?:\/\//, '');
}
