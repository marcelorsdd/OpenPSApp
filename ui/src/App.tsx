import { useTranslation } from 'react-i18next';
import { LanguagePicker, Pill } from './components/ui';
import { useLanguageChoice } from './i18n/useLanguage';
import { href, useRoute } from './router';
import GamePage from './routes/GamePage';
import Profile from './routes/Profile';
import SetPage from './routes/SetPage';
import Trophies from './routes/Trophies';

export default function App() {
  const route = useRoute();
  const { t } = useTranslation();
  const { choice, setChoice } = useLanguageChoice();
  const tab = route.name === 'trophies' || route.name === 'set' ? 'trophies' : 'profile';

  return (
    <div className="max-w-5xl mx-auto px-8 pb-16">
      <header className="flex items-center justify-between py-8">
        <a href={href.profile()} className="text-2xl font-bold tracking-tight">
          PlayTrack
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

      {route.name === 'profile' && <Profile />}
      {route.name === 'trophies' && <Trophies />}
      {route.name === 'game' && <GamePage titleId={route.titleId} />}
      {route.name === 'set' && <SetPage setId={route.setId} />}
    </div>
  );
}
