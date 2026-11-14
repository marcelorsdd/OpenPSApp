import { CircleAlertIcon, QrCodeIcon } from 'lucide-react';
import { Controller, type UseFormReturn, useWatch } from 'react-hook-form';
import { useTranslation } from 'react-i18next';
import { Alert, AlertDescription, AlertTitle } from '@/components/ui/alert';
import { Button } from '@/components/ui/button';
import { Field, FieldDescription, FieldLabel } from '@/components/ui/field';
import { InputOTP, InputOTPGroup, InputOTPSeparator, InputOTPSlot } from '@/components/ui/input-otp';
import { Spinner } from '@/components/ui/spinner';
import type { LinkCodeValues } from '../_schemas/link-code.schema';
import type { CodeError } from '../_types/link';

const SLOTS = [
  [0, 1, 2, 3],
  [4, 5, 6, 7],
];

function ErrorAlert({ error }: { error: CodeError }) {
  const { t } = useTranslation();
  return (
    <Alert variant="destructive" className="mt-4">
      <CircleAlertIcon />
      {error === 'invalid' ? (
        <>
          <AlertTitle>{t('link.invalid.title')}</AlertTitle>
          <AlertDescription>{t('link.invalid.description')}</AlertDescription>
        </>
      ) : (
        <AlertTitle>{error === 'rateLimited' ? t('link.rateLimited') : t('link.failed')}</AlertTitle>
      )}
    </Alert>
  );
}

export function CodeStep({
  form,
  sanitize,
  codeLength,
  error,
  pending,
  canScan,
  scanOnPhone,
  onSubmit,
  onRetry,
  onScan,
}: {
  form: UseFormReturn<LinkCodeValues>;
  sanitize: (raw: string) => string;
  codeLength: number;
  error: CodeError | undefined;
  pending: boolean;
  canScan: boolean;
  scanOnPhone: boolean;
  onSubmit: () => void;
  onRetry: () => void;
  onScan: () => void;
}) {
  const { t } = useTranslation();
  const code = useWatch({ control: form.control, name: 'code' });
  const invalid = error === 'invalid' || !!form.formState.errors.code;
  const complete = code.length === codeLength;
  const action =
    error === 'invalid'
      ? { label: t('common.retry'), onClick: onRetry, disabled: false }
      : { label: null, onClick: onSubmit, disabled: !complete || pending };
  return (
    <section className="basis-form md:rounded-2xl md:border md:border-border md:bg-card md:p-8">
      <h1 className="text-2xl font-extrabold md:text-title md:font-bold">
        <span className="md:hidden">{t('link.phoneTitle')}</span>
        <span className="max-md:hidden">{t('link.title')}</span>
      </h1>
      <p className="mt-2 text-muted-foreground md:mt-1.5">
        <span className="md:hidden">{t('link.phoneHint')}</span>
        <span className="max-md:hidden">{t('link.hint')}</span>
      </p>
      {scanOnPhone && (
        <div className="md:hidden">
          <Button size="xl" className="mt-6 h-14 w-full text-base font-bold" onClick={onScan}>
            <QrCodeIcon className="size-5" />
            {t('link.scan')}
          </Button>
          <div className="mt-6 flex items-center gap-3 text-caption text-muted-foreground">
            <span className="h-px flex-1 bg-border" />
            {t('link.orType')}
            <span className="h-px flex-1 bg-border" />
          </div>
        </div>
      )}
      <form
        noValidate
        onSubmit={(event) => {
          event.preventDefault();
          action.onClick();
        }}
        className="mt-4 md:mt-6"
      >
        <Controller
          control={form.control}
          name="code"
          render={({ field }) => (
            <Field data-invalid={invalid} className="gap-2">
              <FieldLabel htmlFor="link-code" className="text-(length:--text-caption) font-medium text-foreground">
                {t('link.codeLabel')}
              </FieldLabel>
              <InputOTP
                id="link-code"
                ref={field.ref}
                maxLength={codeLength}
                value={field.value}
                onChange={(value) => {
                  if (error) form.clearErrors('root');
                  field.onChange(sanitize(value));
                }}
                onBlur={field.onBlur}
                pasteTransformer={(pasted) => sanitize(pasted)}
                inputMode="text"
                autoCapitalize="characters"
                autoComplete="off"
                spellCheck={false}
                disabled={pending}
                containerClassName="gap-1.5 md:gap-2.5"
                aria-invalid={invalid}
              >
                {SLOTS.map((group, index) => (
                  <span key={group[0]} className="contents">
                    {index > 0 && <InputOTPSeparator />}
                    <InputOTPGroup>
                      {group.map((slot) => (
                        <InputOTPSlot key={slot} index={slot} aria-invalid={invalid} className="text-foreground" />
                      ))}
                    </InputOTPGroup>
                  </span>
                ))}
              </InputOTP>
              <FieldDescription className="text-(length:--text-caption)">{t('link.expires')}</FieldDescription>
            </Field>
          )}
        />
        {error && <ErrorAlert error={error} />}
        <Button type="submit" size="lg" disabled={action.disabled} className="mt-6 w-full max-md:hidden">
          {pending && <Spinner />}
          {action.label ?? t('link.continue')}
        </Button>
        <div className="fixed inset-x-4 bottom-9 z-30 md:hidden">
          <Button
            type="submit"
            size="xl"
            variant={scanOnPhone ? 'outline' : 'default'}
            disabled={action.disabled}
            className="h-12.5 w-full"
          >
            {pending && <Spinner />}
            {action.label ?? (scanOnPhone ? t('link.continuePhone') : t('link.continue'))}
          </Button>
        </div>
        {canScan && (
          <Button type="button" variant="outline" size="lg" className="mt-3 w-full max-md:hidden" onClick={onScan}>
            <QrCodeIcon />
            {t('link.scan')}
          </Button>
        )}
      </form>
    </section>
  );
}
