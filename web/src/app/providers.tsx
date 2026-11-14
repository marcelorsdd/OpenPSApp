import { ClerkProvider } from '@clerk/react';
import type { Health } from '@openpsapp/contract/api';
import { useQuery } from '@tanstack/react-query';
import type { ReactNode } from 'react';
import { useTranslation } from 'react-i18next';
import { router } from '@/app/router';
import { LogoMark } from '@/components/brand/logo';
import { SplashScreen } from '@/components/splash-screen';
import { Button } from '@/components/ui/button';
import { Toaster } from '@/components/ui/sonner';
import { QueryProvider } from '@/lib/query-client';
import { queryKeys } from '@/lib/query-keys';

const appearance = {
  variables: {
    colorPrimary: 'var(--primary)',
    colorPrimaryForeground: 'var(--primary-foreground)',
    colorBackground: 'var(--card)',
    colorForeground: 'var(--foreground)',
    colorMutedForeground: 'var(--muted-foreground)',
    colorMuted: 'var(--muted)',
    colorInput: 'var(--muted)',
    colorInputForeground: 'var(--foreground)',
    colorNeutral: 'var(--foreground)',
    colorBorder: 'var(--border)',
    colorDanger: 'var(--destructive)',
    colorSuccess: 'var(--success)',
    colorWarning: 'var(--warning)',
    borderRadius: 'var(--radius-lg)',
    fontFamily: 'var(--font-sans)',
  },
};

async function fetchHealth(): Promise<Health> {
  const response = await fetch('/api/v1/health');
  if (!response.ok) throw new Error(`HTTP ${response.status}`);
  return (await response.json()) as Health;
}

function StartupError({ message, onRetry }: { message: string; onRetry: () => void }) {
  const { t } = useTranslation();
  return (
    <div className="flex min-h-dvh flex-col items-center justify-center gap-5 px-4 text-center">
      <LogoMark className="size-14" />
      <p className="max-w-form text-muted-foreground">{message}</p>
      <Button onClick={onRetry}>{t('common.retry')}</Button>
    </div>
  );
}

function AuthProvider({ children }: { children: ReactNode }) {
  const { t } = useTranslation();
  const health = useQuery({ queryKey: queryKeys.health, queryFn: fetchHealth, staleTime: Number.POSITIVE_INFINITY });
  if (health.isPending) return <SplashScreen />;
  const key = health.data?.clerkPublishableKey;
  if (!key) {
    return <StartupError message={health.isError ? t('startup.offline') : t('startup.missingKey')} onRetry={() => void health.refetch()} />;
  }
  return (
    <ClerkProvider
      publishableKey={key}
      appearance={appearance}
      afterSignOutUrl="/"
      routerPush={(to) => router.navigate(to)}
      routerReplace={(to) => router.navigate(to, { replace: true })}
    >
      {children}
    </ClerkProvider>
  );
}

export function Providers({ children }: { children: ReactNode }) {
  return (
    <QueryProvider>
      <AuthProvider>{children}</AuthProvider>
      <Toaster position="bottom-right" />
    </QueryProvider>
  );
}
