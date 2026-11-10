import { useTranslation } from 'react-i18next';
import { href } from '@/utils/router';

export default function BackLink() {
  const { t } = useTranslation();
  const back = (e: { preventDefault: () => void }) => {
    if (window.history.length > 1) {
      e.preventDefault();
      window.history.back();
    }
  };
  return (
    <a href={href.profile()} onClick={back} className="inline-block text-muted hover:text-ink mb-6">
      {t('common.back')}
    </a>
  );
}
