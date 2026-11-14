import { useState } from 'react';
import { readStored, writeStored } from '@/lib/storage';

export function useStoredState<T>(key: string, fallback: T): [T, (value: T) => void] {
  const [value, setValue] = useState<T>(() => readStored(key, fallback));
  const update = (next: T) => {
    setValue(next);
    writeStored(key, next);
  };
  return [value, update];
}
