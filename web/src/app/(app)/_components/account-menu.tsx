import { useTranslation } from 'react-i18next';
import {
  DropdownMenu,
  DropdownMenuContent,
  DropdownMenuGroup,
  DropdownMenuItem,
  DropdownMenuLabel,
  DropdownMenuSeparator,
  DropdownMenuTrigger,
} from '@/components/ui/dropdown-menu';

export function AccountMenu({
  initial,
  name,
  email,
  whatIsSentUrl,
  onAccount,
  onSignOut,
}: {
  initial: string;
  name: string | null;
  email: string | null;
  whatIsSentUrl: string;
  onAccount: () => void;
  onSignOut: () => void;
}) {
  const { t } = useTranslation();
  return (
    <DropdownMenu>
      <DropdownMenuTrigger
        aria-label={t('header.accountMenu')}
        className="flex size-(--control-md) items-center justify-center rounded-full bg-primary font-semibold text-primary-foreground outline-none focus-visible:ring-3 focus-visible:ring-ring/30 data-popup-open:ring-3 data-popup-open:ring-ring/30"
      >
        {initial}
      </DropdownMenuTrigger>
      <DropdownMenuContent align="end" sideOffset={8} className="w-60">
        <DropdownMenuGroup>
          <DropdownMenuLabel className="flex flex-col gap-0.5">
            <span className="text-sm font-semibold text-foreground">{name}</span>
            {email && <span className="truncate text-xs font-normal text-muted-foreground">{email}</span>}
          </DropdownMenuLabel>
        </DropdownMenuGroup>
        <DropdownMenuSeparator />
        <DropdownMenuItem onClick={onAccount}>{t('nav.accountAndConsole')}</DropdownMenuItem>
        <DropdownMenuItem render={<a href={whatIsSentUrl} target="_blank" rel="noreferrer" />}>{t('header.whatIsSent')}</DropdownMenuItem>
        <DropdownMenuSeparator />
        <DropdownMenuItem onClick={onSignOut}>{t('header.signOut')}</DropdownMenuItem>
      </DropdownMenuContent>
    </DropdownMenu>
  );
}
