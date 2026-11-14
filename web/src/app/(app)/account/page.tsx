import { useClerk } from '@clerk/react';
import { useTranslation } from 'react-i18next';
import { ErrorState } from '@/components/error-state';
import { PageLoader } from '@/components/page-loader';
import { useDevices } from '@/hooks/use-devices';
import { useProfile } from '@/hooks/use-profile';
import { paths, SYNC_DOC_URL } from '@/lib/links';
import { currentDevice } from '../_lib/sync-status';
import { ConsoleCard } from './_components/console-card';
import { DeleteAccountDialog } from './_components/delete-account-dialog';
import { DisconnectDialog } from './_components/disconnect-dialog';
import { NoConsoleCard } from './_components/no-console-card';
import { PreferencesCard } from './_components/preferences-card';
import { PrivacyCard } from './_components/privacy-card';
import { SessionCard } from './_components/session-card';
import { useDeleteAccount } from './_hooks/use-delete-account';
import { useDisconnect } from './_hooks/use-disconnect';
import { useLanguageChoice } from './_hooks/use-language-choice';
import { accountTotals, languageOptions } from './_lib/describe-account';

export default function AccountPage() {
  const { t } = useTranslation();
  const clerk = useClerk();
  const devices = useDevices();
  const profile = useProfile();
  const device = currentDevice(devices.data);
  const disconnect = useDisconnect(device?.id ?? null);
  const deleteAccount = useDeleteAccount();
  const language = useLanguageChoice();

  if (devices.isError) return <ErrorState onRetry={() => void devices.refetch()} />;
  if (!devices.data) return <PageLoader />;

  return (
    <div className="mx-auto flex max-w-narrow flex-col gap-3 px-4 pt-5 pb-16 sm:px-6 md:gap-4 md:pt-10 lg:px-12">
      <h1 className="text-heading font-extrabold md:font-bold">{t('account.title')}</h1>
      {device ? (
        <ConsoleCard
          device={device}
          totals={accountTotals(profile.data)}
          changeTo={paths.link}
          onDisconnect={() => disconnect.setOpen(true)}
        />
      ) : (
        <NoConsoleCard linkTo={paths.link} />
      )}
      <PreferencesCard
        options={languageOptions(t('account.preferences.auto'))}
        language={language.choice}
        onLanguageChange={language.setChoice}
      />
      <PrivacyCard whatIsSentUrl={SYNC_DOC_URL} onDelete={() => deleteAccount.setOpen(true)} />
      <SessionCard onSignOut={() => void clerk.signOut({ redirectUrl: paths.profile })} />
      {device && (
        <DisconnectDialog
          open={disconnect.open}
          onOpenChange={disconnect.setOpen}
          name={device.profileName}
          purge={disconnect.purge}
          onPurgeChange={disconnect.setPurge}
          pending={disconnect.pending}
          onConfirm={disconnect.confirm}
        />
      )}
      <DeleteAccountDialog
        open={deleteAccount.open}
        onOpenChange={deleteAccount.setOpen}
        pending={deleteAccount.pending}
        onConfirm={deleteAccount.confirm}
      />
    </div>
  );
}
