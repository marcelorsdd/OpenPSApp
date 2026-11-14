export const GITHUB_URL = 'https://github.com/marcelorsdd/OpenPSApp';
export const SYNC_DOC_URL = `${GITHUB_URL}/blob/main/SYNC.md`;

export const paths = {
  profile: '/',
  trophies: '/trophies',
  account: '/account',
  link: '/link',
  game: (titleId: string) => `/games/${encodeURIComponent(titleId)}`,
};
