export type Route =
  | { name: 'profile' }
  | { name: 'trophies' }
  | { name: 'game'; titleId: string }
  | { name: 'set'; setId: string };
