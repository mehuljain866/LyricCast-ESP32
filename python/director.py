"""
LyricCast Semantic Director & Storyboard Generator
Analyzes lyric lines, detects semantic motion metaphors, emotional keywords,
assigns typographic hierarchy, hand-drawn doodles, and animation directives.
"""

import re
import random

# Keyword & Metaphor Dictionaries
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
        self.last_theme = "handwritten_notebook"

    def reset_song(self, song_title):
        self.chorus_counts = {}
        self.song_history = []
        self.last_theme = "handwritten_notebook"

    def analyze_line(self, line_text, duration=3.0, song_position=0.0):
        clean_text = line_text.strip()
        words = re.findall(r"[A-Za-z0-9'’]+|[^\w\s]", clean_text)
        
        if not words or clean_text in ["...", "♫", "♥", "★", "☺"]:
            return {
                "type": "idle",
                "text": clean_text,
                "focal_word": "",
                "metaphor": "idle",
                "doodle": "none",
                "style": "minimal",
                "hierarchy": [{"text": clean_text, "role": "idle", "scale": 1.0, "font": "handwritten"}]
            }

        # 1. Detect Motion Metaphor
        detected_metaphor = "normal"
        for meta, triggers in METAPHORS.items():
            for w in words:
                if w.lower() in triggers:
                    detected_metaphor = meta
                    break
            if detected_metaphor != "normal":
                break

        # 2. Detect Doodle Keyword
        detected_doodle = "none"
        doodle_target_word = ""
        for doodle_type, triggers in DOODLE_MAP.items():
            for w in words:
                if w.lower() in triggers:
                    detected_doodle = doodle_type
                    doodle_target_word = w
                    break
            if detected_doodle != "none":
                break

        # If no explicit doodle keyword, randomly select accents for emotional lines
        if detected_doodle == "none":
            if len(words) > 3 and random.random() < 0.35:
                detected_doodle = random.choice(["underline", "star", "arrow"])

        # 3. Determine Typographic Hierarchy (Focal Word Selection)
        focal_index = -1
        max_score = -1

        for idx, w in enumerate(words):
            low_w = w.lower()
            if low_w in STOP_WORDS or len(w) < 2:
                continue
            
            score = len(w) * 1.5
            if low_w in triggers or (detected_metaphor != "normal" and low_w in METAPHORS.get(detected_metaphor, [])):
                score += 15.0
            if detected_doodle != "none" and low_w in DOODLE_MAP.get(detected_doodle, []):
                score += 12.0
            if w.isupper() and len(w) > 1:
                score += 10.0
            
            # Prefer middle/late words for dramatic effect
            position_bias = (idx / max(1, len(words) - 1)) * 3.0
            score += position_bias

            if score > max_score:
                max_score = score
                focal_index = idx

        if focal_index == -1:
            focal_index = len(words) // 2

        # 4. Build Structured Typographic Segments
        focal_word = words[focal_index] if 0 <= focal_index < len(words) else ""
        
        prefix_words = words[:focal_index]
        suffix_words = words[focal_index + 1:] if focal_index + 1 < len(words) else []

        prefix_str = " ".join(prefix_words).strip()
        suffix_str = " ".join(suffix_words).strip()

        # Decide animation styles based on metaphor
        anim_behavior = "slide_pop"
        if detected_metaphor == "falling":
            anim_behavior = "gravity_drop"
        elif detected_metaphor == "flying":
            anim_behavior = "float_up"
        elif detected_metaphor == "spinning":
            anim_behavior = "wobble_rotate"
        elif detected_metaphor == "running":
            anim_behavior = "fast_zoom"
        elif detected_metaphor == "broken":
            anim_behavior = "split_apart"
        elif detected_metaphor == "alone":
            anim_behavior = "isolated_fade"
        elif detected_metaphor == "scream":
            anim_behavior = "huge_shake"
        elif detected_metaphor == "shake":
            anim_behavior = "jitter"

        # Construct Director Scene Recipe
        scene = {
            "type": "kinetic_scene",
            "raw_text": clean_text,
            "prefix": prefix_str,
            "focal_word": focal_word,
            "suffix": suffix_str,
            "metaphor": detected_metaphor,
            "doodle": detected_doodle,
            "doodle_target": doodle_target_word or focal_word,
            "anim_behavior": anim_behavior,
            "tilt_angle": random.choice([-5, -3, 0, 3, 5, 7]) if detected_metaphor != "alone" else 0,
            "has_underline": (detected_doodle == "underline" or random.random() < 0.4) and len(focal_word) > 2,
            "scale_mode": "huge_focal" if len(clean_text) < 40 else "balanced_notebook"
        }

        return scene
