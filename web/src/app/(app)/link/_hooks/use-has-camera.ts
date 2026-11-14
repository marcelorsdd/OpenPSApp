import { useEffect, useState } from 'react';

export function useHasCamera(): boolean {
  const [hasCamera, setHasCamera] = useState(false);
  useEffect(() => {
    let cancelled = false;
    navigator.mediaDevices
      ?.enumerateDevices()
      .then((devices) => {
        if (!cancelled) setHasCamera(devices.some((device) => device.kind === 'videoinput'));
      })
      .catch(() => undefined);
    return () => {
      cancelled = true;
    };
  }, []);
  return hasCamera;
}
