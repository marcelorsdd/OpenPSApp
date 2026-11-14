import { sanitizeTypedCode, USER_CODE_LENGTH } from '@openpsapp/core';
import { paths, SYNC_DOC_URL } from '@/lib/links';
import { CodeHelp } from './_components/code-help';
import { CodeStep } from './_components/code-step';
import { ConfirmStep } from './_components/confirm-step';
import { DoneStep } from './_components/done-step';
import { ScannerStep } from './_components/scanner-step';
import { useFirstSync } from './_hooks/use-first-sync';
import { useHasCamera } from './_hooks/use-has-camera';
import { useLinkFlow } from './_hooks/use-link-flow';
import { useQrScanner } from './_hooks/use-qr-scanner';
import { displayCode } from './_lib/describe-link';

export default function LinkPage() {
  const flow = useLinkFlow();
  const hasCamera = useHasCamera();
  const scanner = useQrScanner({ active: flow.step.kind === 'scan', onValue: flow.acceptScan });
  const sync = useFirstSync(flow.step.kind === 'done' ? flow.step.previousDeviceIds : null);
  const { step } = flow;

  return (
    <div className="mx-auto flex max-w-link flex-wrap items-center justify-center gap-12 px-4 pt-2 pb-36 sm:px-6 md:py-18 lg:px-12">
      {step.kind === 'confirm' ? (
        <ConfirmStep
          profileName={step.profileName}
          code={displayCode(step.code)}
          replaces={step.replaces}
          whatIsSentUrl={SYNC_DOC_URL}
          canScan={hasCamera}
          pending={flow.confirming}
          onConfirm={flow.confirm}
          onOtherCode={flow.otherCode}
          onScanAgain={flow.scan}
        />
      ) : step.kind === 'done' ? (
        <DoneStep profileName={step.profileName} phase={sync.phase} step={sync.step} steps={sync.steps} profileTo={paths.profile} />
      ) : (
        <>
          <CodeStep
            form={flow.code.form}
            sanitize={sanitizeTypedCode}
            codeLength={USER_CODE_LENGTH}
            error={flow.code.rootError}
            pending={flow.code.pending}
            canScan={hasCamera}
            scanOnPhone
            onSubmit={() => void flow.code.submit()}
            onRetry={flow.code.reset}
            onScan={flow.scan}
          />
          <CodeHelp />
        </>
      )}
      {step.kind === 'scan' && (
        <ScannerStep
          videoRef={scanner.videoRef}
          status={scanner.status}
          rejected={scanner.rejected}
          onClose={flow.typeCode}
          onTypeCode={flow.typeCode}
          onRetry={scanner.retry}
        />
      )}
    </div>
  );
}
