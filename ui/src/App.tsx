import { useTranslation } from 'react-i18next';
import LanguagePicker from '@/components/ui/LanguagePicker';
import Pill from '@/components/ui/Pill';
import { useLanguageChoice } from '@/hooks/useLanguageChoice';
import { useRoute } from '@/hooks/useRoute';
import GamePage from '@/pages/GamePage';
import ProfilePage from '@/pages/ProfilePage';
import SetPage from '@/pages/SetPage';
import TrophiesPage from '@/pages/TrophiesPage';
import { href } from '@/utils/router';

export default function App() {
  const route = useRoute();
  const { t } = useTranslation();
  const { choice, setChoice } = useLanguageChoice();
  const tab = route.name === 'trophies' || route.name === 'set' ? 'trophies' : 'profile';

  return (
    <div className="max-w-5xl mx-auto px-8 pb-16">
      <header className="flex items-center justify-between py-8">
        <a href={href.profile()} className="text-2xl font-bold tracking-tight">
          OpenPSApp
        </a>
        <nav className="flex items-center -mx-1">
          <Pill href={href.profile()} active={tab === 'profile'}>
            {t('nav.profile')}
          </Pill>
          <Pill href={href.trophies()} active={tab === 'trophies'}>
            {t('nav.trophies')}
          </Pill>
          <LanguagePicker value={choice} onChange={setChoice} />
        </nav>
      </header>

      {route.name === 'profile' && <ProfilePage />}
      {route.name === 'trophies' && <TrophiesPage />}
      {route.name === 'game' && <GamePage titleId={route.titleId} />}
      {route.name === 'set' && <SetPage setId={route.setId} />}
    </div>
  );
}
