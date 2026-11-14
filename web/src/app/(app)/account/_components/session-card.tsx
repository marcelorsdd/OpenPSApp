import { LogOutIcon } from 'lucide-react';
import { useTranslation } from 'react-i18next';
import { Button } from '@/components/ui/button';

export function SessionCard({ onSignOut }: { onSignOut: () => void }) {
  const { t } = useTranslation();
  return (
    <Button
      variant="outline"
      className="h-(--control-xl) w-full justify-start rounded-xl border-border px-4.5 md:hidden"
      onClick={onSignOut}
    >
      <LogOutIcon />
      {t('account.signOut')}
    </Button>
  );
}
