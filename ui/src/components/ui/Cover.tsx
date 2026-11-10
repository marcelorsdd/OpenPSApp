export default function Cover({ src, size }: { src: string | null; size: string }) {
  if (!src) return <div className={`${size} rounded-xl bg-raised flex-shrink-0`} aria-hidden="true" />;
  return <img src={src} alt="" className={`${size} rounded-xl object-cover flex-shrink-0 bg-raised`} loading="lazy" />;
}
