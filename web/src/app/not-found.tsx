import { useTranslation } from 'react-i18next';
import { Link } from 'react-router';
import { Logo } from '@/components/brand/logo';
import { Button } from '@/components/ui/button';
import { paths } from '@/lib/links';

export default function NotFoundPage() {
  const { t } = useTranslation();
  return (
    <div className="flex min-h-dvh flex-col">
      <header className="px-4 py-4 sm:px-6 lg:px-12">
        <Link to={paths.profile} aria-label={t('header.home')}>
          <Logo />
        </Link>
      </header>
      <main className="flex flex-1 flex-col items-center justify-center gap-3 px-4 pb-24 text-center">
        <p className="text-display font-extrabold text-muted-foreground">404</p>
        <h1 className="text-heading font-bold">{t('notFound.title')}</h1>
        <p className="text-muted-foreground">{t('notFound.description')}</p>
        <Button size="lg" className="mt-4" render={<Link to={paths.profile} />} nativeButton={false}>
          {t('notFound.home')}
        </Button>
      </main>
    </div>
  );
}
