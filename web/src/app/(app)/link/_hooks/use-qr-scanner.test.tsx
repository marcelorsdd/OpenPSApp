import { render, waitFor } from '@testing-library/react';
import { afterEach, beforeEach, describe, expect, it, vi } from 'vitest';
import type { BarcodeResult } from '../_types/link';
import { useQrScanner } from './use-qr-scanner';

const camera = vi.hoisted(() => ({ track: { stop: vi.fn() }, getUserMedia: vi.fn(), detect: vi.fn() }));

beforeEach(() => {
  camera.getUserMedia.mockResolvedValue({ getTracks: () => [camera.track] });
  vi.stubGlobal('navigator', { ...navigator, mediaDevices: { getUserMedia: camera.getUserMedia } });
  vi.stubGlobal(
    'BarcodeDetector',
    class {
      static getSupportedFormats = () => Promise.resolve(['qr_code']);
      detect = camera.detect;
    },
  );
  vi.spyOn(HTMLMediaElement.prototype, 'play').mockResolvedValue();
});

afterEach(() => {
  vi.clearAllMocks();
  vi.restoreAllMocks();
  vi.unstubAllGlobals();
});

function detects(...values: string[]) {
  for (const value of values) camera.detect.mockResolvedValueOnce([{ rawValue: value }] satisfies BarcodeResult[]);
  camera.detect.mockResolvedValue([]);
}

type Scanner = ReturnType<typeof useQrScanner>;

function renderScanner(onValue: (value: string) => boolean, active = true) {
  const latest: { current: Scanner | null } = { current: null };
  function Harness({ active }: { active: boolean }) {
    const scanner = useQrScanner({ active, onValue });
    latest.current = scanner;
    return <video ref={scanner.videoRef} />;
  }
  const view = render(<Harness active={active} />);
  return { ...view, scanner: () => latest.current!, setActive: (next: boolean) => view.rerender(<Harness active={next} />) };
}

describe('useQrScanner', () => {
  it('keeps scanning past codes that are not ours and stops the camera on the first accepted one', async () => {
    detects('https://example.com', 'ABCD2345');
    const onValue = vi.fn((value: string) => value === 'ABCD2345');
    const { scanner } = renderScanner(onValue);

    await waitFor(() => expect(scanner().status).toBe('scanning'));
    await waitFor(() => expect(scanner().rejected).toBe(true));
    await waitFor(() => expect(onValue).toHaveBeenCalledWith('ABCD2345'));
    expect(camera.track.stop).toHaveBeenCalledTimes(1);

    const reads = camera.detect.mock.calls.length;
    await new Promise((resolve) => setTimeout(resolve, 600));
    expect(camera.detect).toHaveBeenCalledTimes(reads);
    expect(onValue).toHaveBeenCalledTimes(2);
  });

  it('releases the camera and stops reading when it unmounts', async () => {
    detects();
    const { scanner, unmount } = renderScanner(() => false);
    await waitFor(() => expect(scanner().status).toBe('scanning'));
    await waitFor(() => expect(camera.detect).toHaveBeenCalled());

    unmount();
    expect(camera.track.stop).toHaveBeenCalledTimes(1);
    const reads = camera.detect.mock.calls.length;
    await new Promise((resolve) => setTimeout(resolve, 600));
    expect(camera.detect).toHaveBeenCalledTimes(reads);
  });

  it('releases the camera when it is turned off', async () => {
    detects();
    const { scanner, setActive } = renderScanner(() => false);
    await waitFor(() => expect(scanner().status).toBe('scanning'));

    setActive(false);
    expect(camera.track.stop).toHaveBeenCalledTimes(1);
  });

  it('reports a denied permission without reading frames', async () => {
    camera.getUserMedia.mockRejectedValue(Object.assign(new Error('denied'), { name: 'NotAllowedError' }));
    const { scanner } = renderScanner(() => false);
    await waitFor(() => expect(scanner().status).toBe('denied'));
    expect(camera.detect).not.toHaveBeenCalled();
  });

  it('does not open the camera while inactive', () => {
    renderScanner(() => false, false);
    expect(camera.getUserMedia).not.toHaveBeenCalled();
  });
});
