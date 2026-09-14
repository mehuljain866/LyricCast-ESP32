"""
LyricCast Emoji Rasterizer (Expressive++ Mode)
Rasterizes any Unicode emoji into a 16x16 1-bit monochrome bitmap (32 bytes).
Includes pixel-perfect overrides for essential iconic glyphs (e.g. solid beating heart).
"""

import os
from PIL import Image, ImageDraw, ImageFont

EMOJI_CACHE = {}
FONT_PATH = r"C:\Windows\Fonts\seguiemj.ttf"
DEFAULT_FONT = None

# Pixel-perfect handcrafted 16x16 1-bit bitmaps for critical symbols
HANDCRAFTED_BITMAPS = {
    # Solid bold heart (100% filled, pristine silhouette for OLED)
    "❤️": "000000001C387E7EFFFFFFFFFFFF7FFE3FFC1FF80FF007E003C0018000000000",
    "❤": "000000001C387E7EFFFFFFFFFFFF7FFE3FFC1FF80FF007E003C0018000000000",
    "💓": "000000001C387E7EFFFFFFFFFFFF7FFE3FFC1FF80FF007E003C0018000000000",
}

def get_font():
    global DEFAULT_FONT
    if DEFAULT_FONT is None:
        try:
            if os.path.exists(FONT_PATH):
                DEFAULT_FONT = ImageFont.truetype(FONT_PATH, 13)
        except Exception:
            pass
        if DEFAULT_FONT is None:
            DEFAULT_FONT = ImageFont.load_default()
    return DEFAULT_FONT

def rasterize_emoji_16x16(emoji_char):
    """
    Renders any Unicode emoji character into a 16x16 1-bit monochrome bitmap.
    Returns:
        (hex_string_64_chars, raw_bytes_32)
    """
    clean_char = emoji_char.strip()
    if clean_char in EMOJI_CACHE:
        return EMOJI_CACHE[clean_char]

    if clean_char in HANDCRAFTED_BITMAPS:
        hex_str = HANDCRAFTED_BITMAPS[clean_char]
        raw_bytes = bytes.fromhex(hex_str)
        EMOJI_CACHE[clean_char] = (hex_str, raw_bytes)
        return hex_str, raw_bytes

    # Strip invisible variation selector-16 (U+FE0F) which miscalculates Pillow textbbox width
    render_char = clean_char.replace("\ufe0f", "")

    img = Image.new('1', (16, 16), color=0)
    draw = ImageDraw.Draw(img)
    font = get_font()

    try:
        bbox = draw.textbbox((0, 0), render_char, font=font)
        w = bbox[2] - bbox[0]
        h = bbox[3] - bbox[1]
        x = (16 - w) // 2 - bbox[0]
        y = (16 - h) // 2 - bbox[1]
        draw.text((x, y), render_char, font=font, fill=1)
    except Exception:
        draw.text((1, 0), render_char, font=font, fill=1)

    raw_bytes = img.tobytes()
    hex_str = raw_bytes.hex().upper()

    EMOJI_CACHE[clean_char] = (hex_str, raw_bytes)
    return hex_str, raw_bytes

