"""
LyricCast Semantic Director & Storyboard Generator (V3)
Analyzes lyric lines, detects semantic motion metaphors, emotional keywords,
assigns typographic hierarchy, hand-drawn doodles, composition strategies, and animation directives.
"""

import re
import random
import hashlib

METAPHORS = {
    'falling': ['fall', 'falling', 'fell', 'drop', 'dropping', 'down', 'sink', 'sinking', 'drown', 'drowning', 'bottom', 'low', 'deep'],
    'flying': ['fly', 'flying', 'flew', 'float', 'floating', 'sky', 'clouds', 'high', 'rise', 'rising', 'up', 'heaven', 'space', 'soar', 'wings', 'angel', 'higher'],
    'spinning': ['spin', 'spinning', 'rotate', 'round', 'around', 'circle', 'spiral', 'twist', 'whirl', 'roll', 'dance', 'dancing', 'dizzy'],
    'running': ['run', 'running', 'ran', 'fast', 'speed', 'chase', 'chasing', 'escape', 'away', 'drive', 'driving', 'hurry', 'rush', 'race', 'rushing'],
    'broken': ['break', 'broken', 'breaking', 'tear', 'tearing', 'apart', 'shatter', 'crush', 'crack', 'pieces', 'split', 'bleed', 'hurt', 'pain', 'scars'],
    'alone': ['alone', 'lonely', 'nobody', 'empty', 'silence', 'quiet', 'hollow', 'isolated', 'lost', 'darkness', 'dark', 'ghost', 'miss'],
    'scream': ['scream', 'screaming', 'shout', 'shouting', 'loud', 'yell', 'noise', 'roar', 'wild', 'crazy', 'mad', 'shook'],
    'stay': ['stay', 'hold', 'never', 'forever', 'always', 'stop', 'stand', 'freeze', 'wait', 'waiting', 'keep'],
    'shake': ['shake', 'shaking', 'tremble', 'vibrate', 'quake', 'nervous', 'scared', 'fear', 'beat', 'pump']
}

# High-priority semantic doodle keywords
DOODLE_MAP = {
    'rose': ['rose', 'roses', 'flower', 'flowers', 'garden', 'petal', 'petals', 'bloom', 'blooming', 'blossom', 'beauty', 'bouquet', 'daisies', 'daisy'],
    'heart': ['love', 'loved', 'loving', 'heart', 'kiss', 'kissing', 'darling', 'baby', 'sweet', 'crush', 'desire', 'passion', 'yours', 'mine', 'romance'],
    'note': ['music', 'song', 'sing', 'singing', 'melody', 'rhythm', 'tune', 'sound', 'radio', 'listen', 'hear', 'loud', 'stereo', 'dance'],
    'fire': ['fire', 'burn', 'burning', 'flame', 'flames', 'hot', 'heat', 'blaze', 'fever', 'smoke', 'wild', 'ignite', 'ashes'],
    'star': ['star', 'stars', 'night', 'moon', 'dream', 'dreams', 'shine', 'shining', 'glow', 'glowing', 'magic', 'spark', 'light', 'bright', 'sky', 'wish', 'glitter'],
    'rain': ['rain', 'raining', 'cry', 'crying', 'tears', 'sad', 'storm', 'wash', 'wet', 'pour', 'drip', 'teardrop', 'teardrops', 'water', 'flood'],
    'broken': ['break', 'broken', 'breaking', 'apart', 'shatter', 'crush', 'crack', 'pieces', 'split', 'bleed', 'hurt'],
    'wings': ['fly', 'flying', 'wings', 'bird', 'birds', 'angel', 'soar', 'feather', 'free', 'freedom', 'butterfly'],
    'sun': ['sun', 'sunny', 'sunrise', 'sunset', 'daylight', 'morning', 'summer', 'gold', 'golden', 'warm', 'warmth', 'shine'],
    'lightning': ['lightning', 'electric', 'shock', 'strike', 'thunder', 'power', 'flash', 'energy', 'zap', 'voltage', 'spark'],
    'eye': ['eye', 'eyes', 'look', 'looking', 'see', 'seeing', 'watch', 'stare', 'gaze', 'blind', 'sight', 'view'],
    'wave': ['wave', 'waves', 'ocean', 'sea', 'pulse', 'heartbeat', 'alive', 'feel', 'feeling', 'vibe', 'current', 'flow', 'flowing'],
    'bubble': ['think', 'thinking', 'thought', 'thoughts', 'mind', 'talk', 'talking', 'say', 'saying', 'speak', 'words', 'tell', 'whisper'],
    'box': ['name', 'title', 'brand', 'label', 'tag', 'number', 'one', 'best', 'king', 'queen', 'legend', 'forever'],
    'circle': ['all', 'everything', 'world', 'forever', 'always', 'one', 'whole', 'complete', 'around'],
    'arrow': ['you', 'me', 'her', 'him', 'them', 'there', 'here', 'point', 'straight', 'direct', 'target']
}

DECORATIVE_DOODLES = ["STAR", "NOTE", "CIRCLE", "UNDERLINE", "WAVE", "BUBBLE", "BOX", "HEART", "ROSE"]

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
        self.last_doodle = ""

    def reset_song(self, song_title, artist=""):
        self.chorus_counts = {}
        self.song_history = []
        self.last_doodle = ""
        
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
                "font_preset": 0,
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

        # 2. Detect Semantic Doodle Keyword
        detected_doodle = "NONE"
        for doodle_type, triggers in DOODLE_MAP.items():
            for w in words:
                if w.lower() in triggers:
                    # Avoid repeating same doodle consecutively
                    cand = doodle_type.upper()
                    if cand != self.last_doodle or len(words) <= 2:
                        detected_doodle = cand
                        break
            if detected_doodle != "NONE":
                break

        # If no explicit doodle keyword, assign varied non-repetitive decoration (NOT just arrow/underline!)
        if detected_doodle == "NONE":
            if len(words) >= 3 and random.random() < 0.60:
                choices = [d for d in DECORATIVE_DOODLES if d != self.last_doodle]
                detected_doodle = random.choice(choices)
            else:
                detected_doodle = "NONE"

        self.last_doodle = detected_doodle

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
                score += 20.0
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
        elif len(clean_text) > 30:
            composition = "STACKED"

        # 5. Dynamic Tilt Angle & Underline (Used judiciously!)
        tilt_angle = 0
        if composition != "ISOLATED":
            tilt_angle = random.choice([-5, -3, 0, 3, 5])
            
        has_underline = (detected_doodle == "UNDERLINE" or (random.random() < 0.25 and detected_doodle in ["NONE", "NOTE"])) and len(focal_word) > 2

        # 6. Font Preset Pairing for Mix Mode (0=ItalicScript, 1=SansBold, 2=SerifBold, 3=MonoBold, 4=Picopixel/Delicate)
        font_preset = random.randint(0, 4)

        scene = {
            "type": "kinetic_scene",
            "text": clean_text,
            "focal_word": focal_word,
            "prefix": prefix_str,
            "suffix": suffix_str,
            "metaphor": detected_metaphor,
            "doodle": detected_doodle,
            "composition": composition,
            "font_preset": font_preset,
            "has_underline": has_underline,
            "tilt_angle": tilt_angle,
            "duration_ms": max(1000, int(duration * 1000))
        }
        
        self.song_history.append(scene)
        return scene

    def format_serial_packet(self, scene):
        if scene["type"] == "idle":
            return f"L|{scene['text']}|\n"
            
        return (
            f"K|{scene['metaphor']}|{scene['doodle']}|{scene['composition']}|"
            f"{scene['focal_word']}|{scene['prefix']}|{scene['suffix']}|"
            f"{scene['tilt_angle']}|{1 if scene['has_underline'] else 0}|"
            f"{scene['duration_ms']}|{scene.get('font_preset', 0)}\n"
        )
