# What OpenPSApp sends to the cloud

OpenPSApp works fully offline on the console. Sending data to the cloud is optional. It starts only when a console profile is linked to an account, on the official server `https://openpsapp.com` or the one set in the `server` key of `/data/openpsapp/config.json`. This page lists everything the console sends, when it sends it, and what it never sends. The message shapes are in [`contract/`](contract/) (MIT).

The console talks only to the server set in `server`. Every request carries the header `User-Agent: OpenPSApp`; after linking, it also carries the console's token.

## Linking a console

1. On the PS5, open OpenPSApp, go to **Account** and choose **Link account**. The console asks the server for a code (`POST /api/v1/devices/codes`), sending only the name of the PS5 profile.
2. The screen shows a short code, such as `K7Q4-PX2A`, and a QR code.
3. On your phone or computer, sign in on the website, type the code (or scan the QR code) and press **Continue**. The confirmation page shows the profile name, and which console it replaces if the account already has one, so you can check that it is your console before you press **Connect**.
4. While the code is on screen, the console asks the server every few seconds whether you confirmed it (`POST /api/v1/devices/token`), sending only the id of its pairing request. Once you confirm, it receives a token of its own and keeps it in `/data/openpsapp/cloud.json`. The server stores only a hash of it.

The code expires after 10 minutes and works once.

To unlink a console, open **Account** on the website and choose **Disconnect**. Its token stops working right away. **Unlink** on the console does the same from the console: it forgets the token at once and, within about 2 seconds, asks the server to revoke it (`DELETE /api/v1/devices/self`). If the server can't be reached, the console keeps the token on a list of tokens to revoke and retries, with growing pauses, until the server confirms; linking again is not held up by it.

An account has one console. Linking a new PS5 replaces the current one: the old console's token stops working, and the old console unlinks itself on its next sync. The data already on the account stays until the new console's first sync replaces it.

The console's token can only send data. It cannot read your account or change anything in it.

## How linking and your data are protected

- **HTTPS with pinned roots.** The console talks to the server only over HTTPS and checks the server's certificate and name. It trusts only the two Let's Encrypt root certificates built into the payload (ISRG Root X1 and X2), not the console's own certificate store; only development builds, whose version ends in `+devca`, also trust a local test CA. Plain `http://` server addresses are refused.
- **A revocable token, stored hashed.** The console's token is a random 256-bit secret. The server keeps only its SHA-256 hash, so a copy of the database does not reveal it, and removing the console on the website revokes it at once (**Unlink** on the console revokes it too, as soon as the server can be reached). On the console the token is kept unencrypted in `/data/openpsapp/cloud.json`, readable only by its owner, but anything that can read the console's files can read it.
- **One console per account.** Linking a new console replaces the old one, and the confirmation page says which console will be replaced.
- **Linking only from the console's own screen.** OpenPSApp also serves its pages to other devices on your network, but it starts, cancels or syncs a link only when the request comes from the PS5 itself (`127.0.0.1`), with a matching `Host` header and a header that other websites cannot send without OpenPSApp's permission. Other devices on your network can see whether the console is linked, the profile name, the time of the last sync, the last sync error and how many seconds a pairing code has left, but never the pairing code or its link. The website asks you to confirm only a code you can see on the PS5 screen at that moment. OpenPSApp also answers only requests addressed to `localhost` or an IP address, never to a host name, so a website cannot use DNS rebinding to read it.
- **No end-to-end encryption.** HTTPS encrypts the data on the way to the server, but the server stores it in a form it can read. That is what lets the website show your games and trophies on any device while the PS5 is off, add up totals and progress, and convert images to WebP. It also means whoever runs the server can technically read the synced data listed below.

## When the console sends data

- When OpenPSApp starts.
- Right after you link the console.
- Every 15 minutes while it runs.
- Within about a minute after a trophy or playtime changes, for example after you earn a trophy. Once a minute the console rebuilds the summary below and compares it with the last one it sent.
- When you choose **Sync now** on the console.

If a sync fails, the console tries again a minute later. Each time, the console first sends a summary. The server answers with what it does not have yet, and the console uploads only that.

## What is sent

**Summary** (`POST /api/v1/sync/manifest`):

- **Profile:** the PS5 profile name and the console language.
- **Each game:** title id, name, total playtime, last-played date, whether you hid it in OpenPSApp, which trophy set it uses, and the ids of its icon and key art. Hidden games are sent too, marked as hidden.
- **Each trophy set:** its id and a version string.
- **Images:** the ids of the images the console can upload.

**Trophy sets** (`PUT /api/v1/sync/sets/<id>`), only when they changed:

- the set name and the id of its icon;
- for each trophy: its number, name, description, grade, whether it is hidden, whether you earned it and when, and the id of its image. Hidden trophies are sent with their real name and description; the website hides them until you reveal them.

**Images** (`PUT /api/v1/sync/assets/<id>`), only when the server does not have them yet:

- game icons, game key art and trophy images. The id is the SHA-256 of the image, so an image already uploaded by anyone is never sent again.

## What is never sent

- Console identifiers (OpenPsId, serial, MAC address).
- PlayStation Network account ids and local user ids.
- Saves, screenshots, videos, or any file other than the images listed above.

Like any HTTPS request, each upload reaches the server from your network's IP address.

## Deleting your data

- **Disconnect** on the website, with **Also delete the account data** checked, unlinks the console and deletes the games, playtime and trophies it sent. Without the box, they stay on the account.
- **Delete account and data** on the website (confirmed with **Delete account**) deletes the account, the linked console, everything it sent and your sign-in.
- Images are stored once per SHA-256 and can be shared by several accounts, so they are not deleted with an account.
- On the console, deleting `/data/openpsapp` removes the token and every OpenPSApp setting.
