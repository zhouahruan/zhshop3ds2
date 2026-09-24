#!/usr/bin/env python3
"""
Generate a proper 3DS BNR1 banner file from a 256x128 PNG.

Output format (0x20840 bytes total):
  - 0x00-0x03: "BNR1" magic
  - 0x04-0x07: reserved (0x00000000)
  - 0x08-0x83F: header with 8 language entries (Japanese, English, French,
                 German, Italian, Spanish, Korean, Portuguese-BR)
    Each entry is 0x100 bytes:
      - 0x80 bytes UTF-16LE short title  (64 chars max)
      - 0x80 bytes UTF-16LE short publisher (64 chars max)
    (We put the title in the English slot only; others stay zero.)
  - 0x840-0x2083F: 256x128 RGBA8 icon (0x20000 bytes)
"""
import struct
import sys
from pathlib import Path

from PIL import Image

BNR_HEADER_SIZE = 0x840
ICON_W, ICON_H = 256, 128
ICON_BYTES = ICON_W * ICON_H * 4  # 0x20000 = 131072
TOTAL_SIZE = BNR_HEADER_SIZE + ICON_BYTES  # 0x20840 = 133184

LANG_ORDER = ["ja", "en", "fr", "de", "it", "es", "ko", "pt-BR"]


def utf16_padded(s: str, byte_len: int) -> bytes:
    """Encode s as UTF-16LE and zero-pad to byte_len bytes (NUL-terminated)."""
    enc = s.encode("utf-16-le")
    if len(enc) >= byte_len:
        enc = enc[: byte_len - 2]  # leave room for NUL
    return enc.ljust(byte_len, b"\x00")


def build_header(title: str, publisher: str) -> bytearray:
    buf = bytearray(BNR_HEADER_SIZE)
    buf[0:4] = b"BNR1"
    buf[4:8] = b"\x00\x00\x00\x00"

    # First language entry starts at offset 0x08.
    # Layout per entry: 0x100 bytes = 0x80 title + 0x80 publisher.
    entry_off = 8
    entry_size = 0x100
    title_bytes = utf16_padded(title, 0x80)
    pub_bytes = utf16_padded(publisher, 0x80)

    # Fill English (index 1) entry; leave others zero.
    en_off = entry_off + 1 * entry_size
    buf[en_off : en_off + 0x80] = title_bytes
    buf[en_off + 0x80 : en_off + 0x100] = pub_bytes

    return buf


def png_to_rgba8(png_path: Path) -> bytes:
    """Load a 256x128 PNG and return 0x20000 bytes of RGBA8 (top-down)."""
    img = Image.open(png_path).convert("RGBA")
    if img.size != (ICON_W, ICON_H):
        img = img.resize((ICON_W, ICON_H), Image.BILINEAR)

    # PIL gives bytes in RGBA order; citro2d/3DS banner expects RGBA8 in
    # memory as 0xAABBGGRR (little-endian u32). Each pixel is stored as a
    # u32 = (A<<24) | (B<<16) | (G<<8) | R. We pack R,G,B,A bytes in that
    # specific order so the bytes on disk are R,G,B,A.
    pixels = list(img.getdata())
    out = bytearray(ICON_BYTES)
    i = 0
    for r, g, b, a in pixels:
        out[i] = r
        out[i + 1] = g
        out[i + 2] = b
        out[i + 3] = a
        i += 4
    return bytes(out)


def main():
    if len(sys.argv) < 3:
        print(f"usage: {sys.argv[0]} <input.png> <output.bin>")
        print("       optional: <title> <publisher>")
        sys.exit(1)

    png_path = Path(sys.argv[1])
    out_path = Path(sys.argv[2])
    title = sys.argv[3] if len(sys.argv) > 3 else "3DS App Store"
    publisher = sys.argv[4] if len(sys.argv) > 4 else "Homebrew"

    header = build_header(title, publisher)
    icon = png_to_rgba8(png_path)

    banner = bytes(header) + icon
    if len(banner) != TOTAL_SIZE:
        raise SystemExit(
            f"banner size mismatch: got {len(banner)}, expected {TOTAL_SIZE}"
        )

    out_path.write_bytes(banner)
    print(f"wrote {out_path} ({len(banner)} bytes)")


if __name__ == "__main__":
    main()
