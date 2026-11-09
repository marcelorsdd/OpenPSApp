import { Pill } from './components/ui';
import { href, useRoute } from './router';
import GamePage from './routes/GamePage';
import Profile from './routes/Profile';
import SetPage from './routes/SetPage';
import Trophies from './routes/Trophies';

export default function App() {
  const route = useRoute();
  const tab = route.name === 'trophies' || route.name === 'set' ? 'trophies' : 'profile';

  return (
    <div className="max-w-5xl mx-auto px-8 pb-16">
      <header className="flex items-center justify-between py-8">
        <a href={href.profile()} className="text-2xl font-bold tracking-tight">
          PlayTrack
        </a>
        <nav className="flex -mx-1">
          <Pill href={href.profile()} active={tab === 'profile'}>
            Perfil
          </Pill>
          <Pill href={href.trophies()} active={tab === 'trophies'}>
            Troféus
          </Pill>
        </nav>
      </header>

      {route.name === 'profile' && <Profile />}
      {route.name === 'trophies' && <Trophies />}
      {route.name === 'game' && <GamePage titleId={route.titleId} />}
      {route.name === 'set' && <SetPage setId={route.setId} />}
    </div>
  );
}
