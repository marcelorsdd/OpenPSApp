import { useClerk, useUser } from '@clerk/react';
import { useLocation, useMatches } from 'react-router';
import { useDevices } from '@/hooks/use-devices';
import { useNow } from '@/hooks/use-now';
import { useProfile } from '@/hooks/use-profile';
import { isFocusedRoute, sectionFor } from '../_lib/navigation';
import { describeSync } from '../_lib/sync-status';

export function useShell() {
  const location = useLocation();
  const matches = useMatches();
  const profile = useProfile();
  const devices = useDevices();
  const { user } = useUser();
  const clerk = useClerk();
  const now = useNow();
  const email = user?.primaryEmailAddress?.emailAddress ?? null;
  const name = profile.data?.user?.name ?? user?.firstName ?? email;
  return {
    section: sectionFor(location.pathname),
    focused: isFocusedRoute(matches),
    name,
    email,
    initial: (name ?? '?').slice(0, 1).toUpperCase(),
    sync: describeSync(devices.data),
    now,
    signOut: () => void clerk.signOut(),
  };
}
