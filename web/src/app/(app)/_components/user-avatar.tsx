import { Avatar, AvatarFallback, AvatarImage } from '@/components/ui/avatar';

export function UserAvatar({
  initial,
  imageUrl,
  className,
  fallbackClassName,
}: {
  initial: string;
  imageUrl: string | null;
  className?: string;
  fallbackClassName?: string;
}) {
  return (
    <Avatar className={className}>
      {imageUrl && <AvatarImage src={imageUrl} alt="" referrerPolicy="no-referrer" />}
      <AvatarFallback className={fallbackClassName}>{initial}</AvatarFallback>
    </Avatar>
  );
}
