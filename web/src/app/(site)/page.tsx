import { useClerk } from '@clerk/react';
import { useTranslation } from 'react-i18next';
import { useLocation } from 'react-router';
import { SiteFooter } from '@/components/layout/site-footer';
import { SiteLinks } from '@/components/layout/site-links';
import { paths, SYNC_DOC_URL } from '@/lib/links';
import { LandingHeader } from './_components/landing-header';
import { LandingHero } from './_components/landing-hero';
import { LandingPreview } from './_components/landing-preview';

export default function LandingPage() {
  const { t } = useTranslation();
  const clerk = useClerk();
  const location = useLocation();
  const back = `${location.pathname}${location.search}${location.hash}`;
  const signIn = () => clerk.openSignIn({ forceRedirectUrl: back, signUpForceRedirectUrl: back });
  const signUp = () => clerk.openSignUp({ forceRedirectUrl: back, signInForceRedirectUrl: back });
  return (
    <div className="flex min-h-dvh flex-col">
      <LandingHeader onSignIn={signIn} onSignUp={signUp} />
      <main className="mx-auto flex w-full max-w-page flex-1 flex-wrap items-center gap-14 px-4 pt-6 pb-16 sm:px-6 md:pt-12 lg:px-12">
        <LandingHero linkNotice={location.pathname === paths.link} whatIsSentUrl={SYNC_DOC_URL} onSignIn={signIn} />
        <LandingPreview />
      </main>
      <SiteFooter start={t('footer.openSource')} end={<SiteLinks github />} />
    </div>
  );
}
