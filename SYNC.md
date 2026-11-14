# What OpenPSApp sends to the cloud

OpenPSApp works fully offline on the console. Sending data to the cloud is optional and starts only after you link a console profile to an account. This page lists everything the console sends, when it sends it, and what it never sends. The message shapes are in [`contract/`](contract/) (MIT).

## Linking a console

1. On the PS5, open OpenPSApp and choose **Conectar conta**. The console asks the server for a code (`POST /api/v1/devices/codes`), sending only the name of the PS5 profile.
2. The screen shows a short code, such as `K7Q4-PX2A`, and a QR code.
3. On your phone or computer, sign in on the website and type the code. The confirmation page shows the profile name so you can check that it is your console.
4. The console then receives a token of its own and keeps it in `/data/openpsapp/`. The server stores only a hash of it.

The code expires after 10 minutes and works once. To unlink a console, remove it under **Dispositivos conectados** on the website. Its token stops working right away.

The console's token can only send data. It cannot read your account or change anything in it.

## When the console sends data

- When OpenPSApp starts.
- Every 15 minutes while it runs.
- Shortly after a trophy or playtime file changes, for example right after you earn a trophy.

Each time, the console first sends a summary. The server answers with what it does not have yet, and the console uploads only that.

## What is sent

**Summary** (`POST /api/v1/sync/manifest`):
- **Profile:** the PS5 profile name and the console language.
- **Each game:** title id, name, total playtime, last-played date, whether you hid it in OpenPSApp, which trophy set it uses, and the ids of its icon and key art.
- **Each trophy set:** its id and a version string.
- **Images:** the ids of the images the console can upload.

**Trophy sets** (`PUT /api/v1/sync/sets/<id>`), only when they changed:
- the set name;
- for each trophy: name, description, grade, whether it is hidden, whether you earned it and when, and the id of its image.

**Images** (`PUT /api/v1/sync/assets/<id>`), only when the server does not have them yet:
- game icons, game key art and trophy images. The id is the SHA-256 of the image, so an icon already uploaded by anyone is never sent again.

## What is never sent

- Console identifiers (OpenPsId, serial, MAC address).
- PlayStation Network account ids and local user ids.
- Saves, screenshots, videos, or any file other than the images listed above.

Like any HTTPS request, each upload reaches the server from your network's IP address.
