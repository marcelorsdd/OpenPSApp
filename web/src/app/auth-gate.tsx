import { useAuth } from '@clerk/react';
import type { ReactNode } from 'react';
import { SplashScreen } from '@/components/splash-screen';

export function AuthGate({ signedIn, signedOut }: { signedIn: ReactNode; signedOut: ReactNode }) {
  const { isLoaded, isSignedIn } = useAuth();
  if (!isLoaded) return <SplashScreen />;
  return isSignedIn ? signedIn : signedOut;
}
