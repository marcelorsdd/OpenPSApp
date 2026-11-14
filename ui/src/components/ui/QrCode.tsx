import { useMemo } from 'react';
import { qrMatrix, qrPath } from '@/utils/qr';

const QUIET = 4;

export default function QrCode({ text, label, className }: { text: string; label: string; className?: string }) {
  const matrix = useMemo(() => qrMatrix(text), [text]);
  const size = matrix.length + QUIET * 2;
  return (
    <svg viewBox={`0 0 ${size} ${size}`} role="img" aria-label={label} shapeRendering="crispEdges" className={className}>
      <rect width={size} height={size} fill="#fff" />
      <path d={qrPath(matrix, QUIET)} fill="#000" />
    </svg>
  );
}
