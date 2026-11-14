import { useClerk } from '@clerk/react';
import { useTranslation } from 'react-i18next';
import { useLocation } from 'react-router';
import { SiteFooter } from '@/components/layout/site-footer';
import { GITHUB_URL, paths, SYNC_DOC_URL } from '@/lib/links';
import { LandingHeader } from './_components/landing-header';
import { LandingHero } from './_components/landing-hero';
import { LandingPreview } from './_components/landing-preview';

export default function LandingPage() {
  const { t } = useTranslation();
  const clerk = useClerk();
  const location = useLocation();
  const back = `${location.pathname}${location.search}`;
  const signIn = () => clerk.openSignIn({ forceRedirectUrl: back, signUpForceRedirectUrl: back });
  const signUp = () => clerk.openSignUp({ forceRedirectUrl: back, signInForceRedirectUrl: back });
  return (
    <div className="flex min-h-dvh flex-col">
      <LandingHeader onSignIn={signIn} onSignUp={signUp} />
      <main className="mx-auto flex w-full max-w-page flex-1 flex-wrap items-center gap-14 px-4 pt-6 pb-16 sm:px-6 md:pt-12 lg:px-12">
        <LandingHero linkNotice={location.pathname === paths.link} whatIsSentUrl={SYNC_DOC_URL} onSignIn={signIn} />
        <LandingPreview />
      </main>
      <SiteFooter
        start={t('footer.openSource')}
        end={
          <>
            <a href={SYNC_DOC_URL} target="_blank" rel="noreferrer" className="text-link hover:text-link-hover">
              {t('footer.privacy')}
            </a>
            <a href={GITHUB_URL} target="_blank" rel="noreferrer" className="text-link hover:text-link-hover">
              {t('footer.github')}
            </a>
          </>
        }
      />
    </div>
  );
}
