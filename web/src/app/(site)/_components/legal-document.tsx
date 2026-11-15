import { useTranslation } from 'react-i18next';
import { Link } from 'react-router';
import { Logo } from '@/components/brand/logo';
import { SiteFooter } from '@/components/layout/site-footer';
import { SiteLinks } from '@/components/layout/site-links';
import { paths } from '@/lib/links';

type LegalSection = { title: string; body: string[]; items?: string[] };

export function LegalDocument({ doc }: { doc: 'privacy' | 'terms' }) {
  const { t } = useTranslation();
  const sections = t(`${doc}.sections`, { returnObjects: true }) as LegalSection[];
  return (
    <div className="flex min-h-dvh flex-col">
      <header className="px-4 py-4 sm:px-6 lg:px-12">
        <Link to={paths.profile} aria-label={t('header.home')}>
          <Logo />
        </Link>
      </header>
      <main className="mx-auto w-full max-w-3xl flex-1 px-4 pt-6 pb-16 sm:px-6">
        <h1 className="text-heading font-bold">{t(`${doc}.title`)}</h1>
        <p className="mt-2 text-muted-foreground">{t(`${doc}.updated`)}</p>
        <p className="mt-6 text-soft-foreground">{t(`${doc}.intro`)}</p>
        {sections.map((section) => (
          <section key={section.title} className="mt-9">
            <h2 className="text-title font-semibold">{section.title}</h2>
            {section.body.map((paragraph) => (
              <p key={paragraph} className="mt-3 text-soft-foreground">
                {paragraph}
              </p>
            ))}
            {section.items && (
              <ul className="mt-3 list-disc space-y-2 pl-5 text-soft-foreground">
                {section.items.map((item) => (
                  <li key={item}>{item}</li>
                ))}
              </ul>
            )}
          </section>
        ))}
      </main>
      <SiteFooter start={t('footer.openSource')} end={<SiteLinks github />} />
    </div>
  );
}
