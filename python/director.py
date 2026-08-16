"""
LyricCast Semantic Director & Storyboard Generator
Analyzes lyric lines, detects semantic motion metaphors, emotional keywords,
assigns typographic hierarchy, hand-drawn doodles, composition strategies, and animation directives.
"""

import re
import random
import hashlib

METAPHORS = {
    'falling': ['fall', 'falling', 'fell', 'drop', 'dropping', 'down', 'sink', 'sinking', 'drown', 'drowning', 'bottom'],
    'flying': ['fly', 'flying', 'flew', 'float', 'floating', 'sky', 'clouds', 'high', 'rise', 'rising', 'up', 'heaven', 'space'],
    'spinning': ['spin', 'spinning', 'rotate', 'round', 'around', 'circle', 'spiral', 'twist', 'whirl', 'roll'],
    'running': ['run', 'running', 'ran', 'fast', 'speed', 'chase', 'chasing', 'escape', 'away', 'drive', 'driving', 'hurry'],
    'broken': ['break', 'broken', 'breaking', 'tear', 'tearing', 'apart', 'shatter', 'crush', 'crack', 'pieces', 'split'],
    'alone': ['alone', 'lonely', 'nobody', 'empty', 'silence', 'quiet', 'hollow', 'isolated', 'lost', 'darkness'],
    'scream': ['scream', 'screaming', 'shout', 'shouting', 'loud', 'yell', 'noise', 'roar', 'wild', 'crazy', 'mad'],
    'stay': ['stay', 'hold', 'never', 'forever', 'always', 'stop', 'stand', 'freeze', 'wait', 'waiting'],
    'shake': ['shake', 'shaking', 'tremble', 'vibrate', 'quake', 'nervous', 'scared', 'fear']
}

DOODLE_MAP = {
    'heart': ['love', 'loved', 'loving', 'heart', 'kiss', 'kissing', 'darling', 'baby', 'sweet', 'crush', 'desire', 'passion'],
    'fire': ['fire', 'burn', 'burning', 'flame', 'flames', 'hot', 'heat', 'blaze', 'fever', 'smoke'],
    'star': ['star', 'stars', 'night', 'moon', 'dream', 'dreams', 'shine', 'shining', 'glow', 'glowing', 'magic', 'spark', 'light', 'bright'],
    'rain': ['rain', 'raining', 'cry', 'crying', 'tears', 'sad', 'storm', 'wash', 'wet', 'pour', 'drip'],
    'lightning': ['lightning', 'electric', 'shock', 'strike', 'thunder', 'power', 'flash', 'energy', 'zap'],
    'cloud': ['cloud', 'clouds', 'fog', 'breeze', 'wind', 'breath', 'air', 'fly'],
    'eye': ['eye', 'eyes', 'look', 'looking', 'see', 'seeing', 'watch', 'stare', 'gaze', 'blind'],
    'arrow': ['you', 'me', 'her', 'him', 'them', 'look', 'there', 'here', 'point', 'straight', 'direct']
}

STOP_WORDS = {
    'a', 'an', 'the', 'and', 'or', 'but', 'if', 'in', 'on', 'at', 'to', 'for', 'with', 'by',
    'about', 'as', 'into', 'like', 'through', 'after', 'over', 'between', 'out', 'against',
    'during', 'without', 'before', 'under', 'around', 'among', 'is', 'am', 'are', 'was', 'were',
    'be', 'been', 'being', 'have', 'has', 'had', 'do', 'does', 'did', 'that', 'this', 'it'
}

class LyricDirector:
    def __init__(self):
        self.chorus_counts = {}
        self.song_history = []
        self.current_seed = 42

    def reset_song(self, song_title, artist=""):
        self.chorus_counts = {}
        self.song_history = []
        
        # Deterministic seed from Song Title & Artist
        seed_str = f"{song_title}-{artist}"
        hash_val = int(hashlib.md5(seed_str.encode('utf-8')).hexdigest()[:8], 16)
        self.current_seed = hash_val & 0xFFFFFFFF
        random.seed(self.current_seed)

    def analyze_line(self, line_text, duration=3.0, song_position=0.0):
        clean_text = line_text.strip()
        words = re.findall(r"[A-Za-z0-9'’]+|[^\w\s]", clean_text)
        
        if not words or clean_text in ["...", "♫", "♥", "★", "☺"]:
            return {
                "type": "idle",
                "text": clean_text,
                "focal_word": "",
                "metaphor": "IDLE",
                "doodle": "NONE",
                "composition": "CENTER",
                "has_underline": False,
                "tilt_angle": 0,
                "duration_ms": int(duration * 1000)
            }

        # 1. Detect Motion Metaphor
        detected_metaphor = "NORMAL"
        for meta, triggers in METAPHORS.items():
            for w in words:
                if w.lower() in triggers:
                    detected_metaphor = meta.upper()
                    break
            if detected_metaphor != "NORMAL":
                break

        # 2. Detect Doodle Keyword
        detected_doodle = "NONE"
        for doodle_type, triggers in DOODLE_MAP.items():
            for w in words:
                if w.lower() in triggers:
                    detected_doodle = doodle_type.upper()
                    break
            if detected_doodle != "NONE":
                break

        # If no explicit doodle keyword, procedurally assign based on length
        if detected_doodle == "NONE":
            if len(words) > 3 and random.random() < 0.40:
                detected_doodle = random.choice(["UNDERLINE", "STAR", "ARROW"])

        # 3. Typographic Hierarchy (Focal Word Selection)
        focal_index = -1
        max_score = -1

        for idx, w in enumerate(words):
            low_w = w.lower()
            if low_w in STOP_WORDS or len(w) < 2:
                continue
            
            score = len(w) * 1.5
            if detected_metaphor != "NORMAL" and low_w in METAPHORS.get(detected_metaphor.lower(), []):
                score += 15.0
            if detected_doodle != "NONE" and low_w in DOODLE_MAP.get(detected_doodle.lower(), []):
                score += 12.0
            if w.isupper() and len(w) > 1:
                score += 10.0
            
            position_bias = (idx / max(1, len(words) - 1)) * 3.0
            score += position_bias

            if score > max_score:
                max_score = score
                focal_index = idx

        if focal_index == -1:
            focal_index = len(words) // 2

        focal_word = words[focal_index] if 0 <= focal_index < len(words) else ""
        prefix_words = words[:focal_index]
        suffix_words = words[focal_index + 1:] if focal_index + 1 < len(words) else []

        prefix_str = " ".join(prefix_words).strip()
        suffix_str = " ".join(suffix_words).strip()

        # 4. Composition Strategy
        composition = "CENTER"
        if detected_metaphor == "ALONE":
            composition = "ISOLATED"
        elif detected_metaphor == "FALLING" or detected_metaphor == "FLYING":
            composition = "DIAGONAL"
        elif len(clean_text) > 35:
            composition = "STACKED"

        # 5. Dynamic Tilt Angle & Underline
        tilt_angle = 0
        if composition != "ISOLATED":
            tilt_angle = random.choice([-5, -3, 0, 3, 5])
            
        has_underline = (detected_doodle == "UNDERLINE" or random.random() < 0.45) and len(focal_word) > 2

        scene = {
            "type": "kinetic_scene",
            "raw_text": clean_text,
            "prefix": prefix_str,
            "focal_word": focal_word,
            "suffix": suffix_str,
            "metaphor": detected_metaphor,
            "doodle": detected_doodle,
            "composition": composition,
            "tilt_angle": tilt_angle,
            "has_underline": has_underline,
            "duration_ms": int(duration * 1000),
            "song_seed": self.current_seed
        }

        return scene
