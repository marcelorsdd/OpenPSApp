#!/usr/bin/env python3
import hashlib
import os
import struct
import sys

ROOT = os.path.dirname(os.path.abspath(__file__))
SEEDS = os.path.join(ROOT, "seeds")


def write(name, index, data):
    d = os.path.join(SEEDS, name)
    os.makedirs(d, exist_ok=True)
    with open(os.path.join(d, f"seed{index:02d}"), "wb") as f:
        f.write(data if isinstance(data, (bytes, bytearray)) else data.encode())


def seeds_url():
    for i, s in enumerate([
        "https://example.com/api?x=1",
        "https://host.example:8443/sync/manifest",
        "https://a",
        "http://example.com/",
    ]):
        write("url", i, s)


def seeds_http_message():
    body = "hello world"
    cl = f"HTTP/1.1 200 OK\r\nContent-Length: {len(body)}\r\n\r\n{body}"
    chunked = "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nhello\r\n0\r\n\r\n"
    nobody = "HTTP/1.1 204 No Content\r\n\r\n"
    for i, s in enumerate([cl, chunked, nobody]):
        write("http_message", i, s)


def seeds_http_request():
    for i, s in enumerate([
        "GET /api/v1/games/PPSA00000 HTTP/1.1\r\nHost: 127.0.0.1:7777\r\n\r\n",
        "GET /api/v1/icons/%2e%2e%2fetc?scale=2 HTTP/1.1\r\nHost: localhost\r\n\r\n",
        "/api/v1/debug/identity?a=b&c=d",
    ]):
        write("http_request", i, s)


def seeds_ucp():
    head = bytearray(0x60)
    struct.pack_into(">I", head, 0, 0xB228C60A)
    struct.pack_into(">I", head, 0x10, 1)
    struct.pack_into(">I", head, 0x14, 64)
    entry = bytearray(64)
    name = b"TROP.ESFM"
    entry[0:len(name)] = name
    struct.pack_into(">Q", entry, 32, 0x60 + 64)
    struct.pack_into(">Q", entry, 40, 0)
    write("ucp", 0, bytes(head + entry))


def t2pd_page(stream):
    pad = (-len(stream)) % 1024
    stream = stream + b"\x00" * pad
    out = bytearray()
    for i in range(0, len(stream), 1024):
        page = stream[i:i + 1024]
        out += page
        out += hashlib.sha256(page).digest()
    return bytes(out), len(stream) // 1024


def seeds_t2pd():
    stream = bytearray(48)
    stream[0:4] = b"T2TD"
    struct.pack_into(">I", stream, 4, 0x10000)
    struct.pack_into(">I", stream, 8, 1)
    dir_entry = bytearray(32)
    struct.pack_into(">I", dir_entry, 0, 0x800)
    struct.pack_into(">I", dir_entry, 4, 0x30)
    struct.pack_into(">I", dir_entry, 8, 1)
    struct.pack_into(">I", dir_entry, 12, 1)
    rec_off = 48 + 32
    struct.pack_into(">Q", dir_entry, 16, rec_off)
    record = bytearray(16 + 0x30)
    struct.pack_into(">I", record, 0, 0x800)
    full = bytes(stream) + bytes(dir_entry) + bytes(record)
    pages, count = t2pd_page(full)
    header = bytearray(64)
    header[0:4] = b"T2PD"
    struct.pack_into(">I", header, 4, 0x10000)
    struct.pack_into(">Q", header, 8, 1024 + 32)
    struct.pack_into(">Q", header, 32, count)
    write("t2pd", 0, bytes(header) + pages)


def seeds_npbind():
    data = bytearray(0x80)
    struct.pack_into(">I", data, 0, 0xD294A018)
    value = b"NPWR00000_00\x00"
    rec = struct.pack(">HH", 0x0010, len(value)) + value
    write("npbind", 0, bytes(data) + rec)


def seeds_timeutil():
    for i, s in enumerate(["1700000000", "1700000000000", "2024-01-02T03:04:05", "20240102030405"]):
        write("timeutil", i, s)


def seeds_config_json():
    write("config_json", 0, '{"port":7777,"language":"en-US","debug":true,"server":"https://x"}')
    write("config_json", 1, "{}")


def seeds_cloud_json():
    write("cloud_json", 0,
          '{"links":{"user":{"token":"abc","deviceId":"d1","profileName":"p"}},"revoke":["tok1"]}')
    write("cloud_json", 1, "{}")


def main():
    for fn in list(globals().values()):
        if callable(fn) and getattr(fn, "__name__", "").startswith("seeds_"):
            fn()


if __name__ == "__main__":
    main()
