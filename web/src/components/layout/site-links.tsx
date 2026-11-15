import { useTranslation } from 'react-i18next';
import { Link } from 'react-router';
import { GITHUB_URL, paths } from '@/lib/links';

const linkClass = 'text-link hover:text-link-hover';

export function SiteLinks({ github = false }: { github?: boolean }) {
  const { t } = useTranslation();
  return (
    <>
      <Link to={paths.privacy} className={linkClass}>
        {t('footer.privacy')}
      </Link>
      <Link to={paths.terms} className={linkClass}>
        {t('footer.terms')}
      </Link>
      {github && (
        <a href={GITHUB_URL} target="_blank" rel="noreferrer" className={linkClass}>
          {t('footer.github')}
        </a>
      )}
    </>
  );
}
