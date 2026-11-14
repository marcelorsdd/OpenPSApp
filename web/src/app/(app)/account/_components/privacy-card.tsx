import { useTranslation } from 'react-i18next';
import { Button } from '@/components/ui/button';
import { Card } from '@/components/ui/card';

export function PrivacyCard({ whatIsSentUrl, onDelete }: { whatIsSentUrl: string; onDelete: () => void }) {
  const { t } = useTranslation();
  return (
    <Card className="gap-4 px-5 md:px-6">
      <div>
        <h2 className="text-body font-semibold">{t('account.privacy.title')}</h2>
        <p className="mt-1.5 text-muted-foreground">{t('account.privacy.description')}</p>
      </div>
      <div className="flex flex-wrap items-center justify-between gap-3">
        <a href={whatIsSentUrl} target="_blank" rel="noreferrer" className="font-medium text-link hover:text-link-hover">
          {t('account.privacy.whatIsSent')}
        </a>
        <Button
          variant="destructive"
          onClick={onDelete}
          className="max-md:h-(--control-xl) max-md:w-full max-md:rounded-xl max-md:bg-destructive-soft max-md:font-semibold max-md:text-destructive-soft-foreground"
        >
          {t('account.privacy.delete')}
        </Button>
      </div>
    </Card>
  );
}
