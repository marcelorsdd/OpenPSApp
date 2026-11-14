import { createBrowserRouter } from 'react-router';
import ProfilePage from '@/app/(app)/(profile)/page';
import AppLayout from '@/app/(app)/layout';
import LandingPage from '@/app/(site)/page';
import { AuthGate } from '@/app/auth-gate';
import RootLayout from '@/app/layout';
import NotFoundPage from '@/app/not-found';
import { SplashScreen } from '@/components/splash-screen';
import type { RouteHandle } from '@/lib/route-handle';

const focused: RouteHandle = { focused: true };

const page = (load: () => Promise<{ default: React.ComponentType }>) => async () => ({ Component: (await load()).default });

export const router = createBrowserRouter([
  {
    element: <RootLayout />,
    hydrateFallbackElement: <SplashScreen />,
    children: [
      {
        element: <AuthGate signedIn={<AppLayout />} signedOut={<LandingPage />} />,
        children: [
          { index: true, element: <ProfilePage /> },
          { path: 'trophies', lazy: page(() => import('@/app/(app)/trophies/page')) },
          { path: 'games/:titleId', lazy: page(() => import('@/app/(app)/games/[titleId]/page')) },
          { path: 'account', lazy: page(() => import('@/app/(app)/account/page')) },
          { path: 'link', handle: focused, lazy: page(() => import('@/app/(app)/link/page')) },
        ],
      },
      { path: '*', element: <NotFoundPage /> },
    ],
  },
]);
