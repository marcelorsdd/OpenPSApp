import { cn } from 'cn';

export function NumberedSteps({ steps, className }: { steps: string[]; className?: string }) {
  return (
    <ol className={cn('flex flex-col gap-3 text-left text-soft-foreground', className)}>
      {steps.map((step, index) => (
        <li key={step} className="flex gap-3">
          <span className="flex size-6 shrink-0 items-center justify-center rounded-full bg-muted text-xs font-bold text-foreground">
            {index + 1}
          </span>
          {step}
        </li>
      ))}
    </ol>
  );
}
