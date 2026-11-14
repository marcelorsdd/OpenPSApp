import { useTranslation } from 'react-i18next';
import AppShell from '@/components/ui/AppShell';
import LanguagePicker from '@/components/ui/LanguagePicker';
import Pill from '@/components/ui/Pill';
import { useApi } from '@/hooks/useApi';
import { useLanguageChoice } from '@/hooks/useLanguageChoice';
import { useRoute } from '@/hooks/useRoute';
import AccountPage from '@/pages/AccountPage';
import GamePage from '@/pages/GamePage';
import ProfilePage from '@/pages/ProfilePage';
import SetPage from '@/pages/SetPage';
import TrophiesPage from '@/pages/TrophiesPage';
import { endpoints } from '@/services/api';
import type { Route } from '@/types/route';
import { href } from '@/utils/router';

function page(route: Route) {
  if (route.name === 'trophies') return <TrophiesPage />;
  if (route.name === 'game') return <GamePage titleId={route.titleId} />;
  if (route.name === 'set') return <SetPage setId={route.setId} />;
  if (route.name === 'account') return <AccountPage />;
  return <ProfilePage />;
}

export default function App() {
  const health = useApi(endpoints.health);
  const language = useLanguageChoice(health.data?.language ?? null);
  const route = useRoute();
  const { t } = useTranslation();
  const tab = route.name === 'trophies' || route.name === 'set' ? 'trophies' : route.name === 'account' ? 'account' : 'profile';

  if (!health.data && !health.error) return null;

  return (
    <AppShell
      nav={
        <>
          <Pill href={href.profile()} active={tab === 'profile'}>
            {t('nav.profile')}
          </Pill>
          <Pill href={href.trophies()} active={tab === 'trophies'}>
            {t('nav.trophies')}
          </Pill>
          <Pill href={href.account()} active={tab === 'account'}>
            {t('nav.account')}
          </Pill>
          <LanguagePicker value={language.choice} onChange={language.setChoice} />
        </>
      }
    >
      {page(route)}
    </AppShell>
  );
}
