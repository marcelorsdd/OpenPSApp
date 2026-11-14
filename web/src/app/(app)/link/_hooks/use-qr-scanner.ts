import { useEffect, useRef, useState } from 'react';
import { cameraStatusFor, centerSquare, createQrDecoder, SCAN_INTERVAL_MS } from '../_lib/qr-decoder';
import type { BarcodeDetectorClass, BarcodeResult, CameraStatus, QrDecoder, QrDecoderDeps } from '../_types/link';

async function loadZxing(): Promise<(frame: unknown) => Promise<BarcodeResult[]>> {
  const [{ prepareZXingModule, readBarcodes }, { default: wasmUrl }] = await Promise.all([
    import('zxing-wasm/reader'),
    import('zxing-wasm/reader/zxing_reader.wasm?url'),
  ]);
  await prepareZXingModule({
    overrides: { locateFile: (path: string, prefix: string) => (path.endsWith('.wasm') ? wasmUrl : prefix + path) },
    fireImmediately: true,
  });
  const canvas = document.createElement('canvas');
  const context = canvas.getContext('2d', { willReadFrequently: true });
  return async (frame) => {
    const video = frame as HTMLVideoElement;
    if (!context || !video.videoWidth) return [];
    const square = centerSquare(video.videoWidth, video.videoHeight);
    canvas.width = square.size;
    canvas.height = square.size;
    context.drawImage(video, square.x, square.y, square.size, square.size, 0, 0, square.size, square.size);
    return readBarcodes(context.getImageData(0, 0, square.size, square.size), {
      formats: ['QRCode'],
      tryHarder: true,
      maxNumberOfSymbols: 1,
    });
  };
}

function browserDeps(): QrDecoderDeps {
  const Detector = (window as unknown as { BarcodeDetector?: BarcodeDetectorClass }).BarcodeDetector;
  return {
    barcodeDetector: Detector
      ? { getSupportedFormats: () => Detector.getSupportedFormats(), create: () => new Detector({ formats: ['qr_code'] }) }
      : null,
    loadZxing,
  };
}

function stopStream(stream: MediaStream | null) {
  stream?.getTracks().forEach((track) => track.stop());
}

export function useQrScanner({ active, onValue }: { active: boolean; onValue: (value: string) => boolean }) {
  const videoRef = useRef<HTMLVideoElement>(null);
  const onValueRef = useRef(onValue);
  const [status, setStatus] = useState<CameraStatus>('starting');
  const [rejected, setRejected] = useState(false);
  const [attempt, setAttempt] = useState(0);

  useEffect(() => {
    onValueRef.current = onValue;
  });

  useEffect(() => {
    if (!active) return;
    let stream: MediaStream | null = null;
    let timer = 0;
    let stopped = false;
    const stop = () => {
      stopped = true;
      window.clearTimeout(timer);
      stopStream(stream);
      stream = null;
      if (videoRef.current) videoRef.current.srcObject = null;
    };
    const scan = async (decode: QrDecoder) => {
      if (stopped || !videoRef.current) return;
      const value = await decode(videoRef.current).catch(() => null);
      if (stopped) return;
      if (value) {
        if (onValueRef.current(value)) {
          stop();
          return;
        }
        setRejected(true);
      }
      timer = window.setTimeout(() => void scan(decode), SCAN_INTERVAL_MS);
    };
    const start = async () => {
      setStatus('starting');
      setRejected(false);
      if (!navigator.mediaDevices?.getUserMedia) {
        setStatus('unavailable');
        return;
      }
      try {
        stream = await navigator.mediaDevices.getUserMedia({ video: { facingMode: { ideal: 'environment' } }, audio: false });
      } catch (error) {
        if (!stopped) setStatus(cameraStatusFor(error));
        return;
      }
      const video = videoRef.current;
      if (stopped || !video) {
        stopStream(stream);
        return;
      }
      try {
        video.srcObject = stream;
        await video.play();
        const decode = await createQrDecoder(browserDeps());
        if (stopped) return;
        setStatus('scanning');
        void scan(decode);
      } catch (error) {
        if (!stopped) {
          stop();
          setStatus(cameraStatusFor(error));
        }
      }
    };
    void start();
    return stop;
  }, [active, attempt]);

  return { videoRef, status, rejected, retry: () => setAttempt((n) => n + 1) };
}
