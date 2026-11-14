import { useTranslation } from 'react-i18next';
import { Logo } from '@/components/brand/logo';
import { Button } from '@/components/ui/button';

export function LandingHeader({ onSignIn, onSignUp }: { onSignIn: () => void; onSignUp: () => void }) {
  const { t } = useTranslation();
  return (
    <header className="flex items-center justify-between gap-4 px-4 py-4 sm:px-6 lg:px-12">
      <Logo className="text-xl max-sm:text-lg" markClassName="size-9" />
      <div className="flex items-center gap-2 sm:gap-3">
        <Button variant="ghost" size="lg" onClick={onSignIn}>
          {t('landing.signIn')}
        </Button>
        <Button size="lg" onClick={onSignUp} className="max-sm:hidden">
          {t('landing.signUp')}
        </Button>
      </div>
    </header>
  );
}
