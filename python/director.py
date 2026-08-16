"""
LyricCast Semantic Director & Storyboard Generator (V6 - 50+ Semantic Doodles & Strict Precision)
Analyzes lyric lines, formats them specifically for the 128x48 Blue OLED zone,
assigns typographic hierarchy across 16 font styles, 50+ precise procedural vector doodles,
8 compositional archetypes, and guarantees zero arbitrary doodle spam.
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
    'scream': ['scream', 'screaming', 'shout', 'shouting', 'loud', 'yell', 'noise', 'roar', 'wild', 'crazy', 'mad', 'shook', 'boom'],
    'stay': ['stay', 'hold', 'never', 'forever', 'always', 'stop', 'stand', 'freeze', 'wait', 'waiting', 'keep'],
    'shake': ['shake', 'shaking', 'tremble', 'vibrate', 'quake', 'nervous', 'scared', 'fear', 'beat', 'pump']
}

# 50+ Precise Semantic Doodle Keyword Categories (Strict Matching, Zero Random Spam)
DOODLE_MAP = {
    'home': ['home', 'house', 'roof', 'room', 'stay', 'door', 'bed', 'walls', 'inside', 'living', 'place', 'hometown'],
    'gun': ['gun', 'guns', 'shot', 'shoot', 'shooting', 'bullet', 'bullets', 'pistol', 'trigger', 'bang', 'aim', 'rifle', 'weapon'],
    'earring': ['ear', 'earring', 'earrings', 'jewelry', 'piercing', 'pierced', 'necklace', 'ring', 'rings'],
    'rose': ['rose', 'roses', 'flower', 'flowers', 'garden', 'petal', 'petals', 'bloom', 'blooming', 'blossom', 'daisies', 'daisy', 'bouquet'],
    'heart': ['love', 'loved', 'loving', 'heart', 'kiss', 'kissing', 'darling', 'baby', 'sweet', 'crush', 'desire', 'passion', 'yours', 'mine', 'romance'],
    'broken': ['break', 'broken', 'breaking', 'apart', 'shatter', 'crush', 'crack', 'pieces', 'split', 'bleed', 'hurt'],
    'note': ['music', 'song', 'sing', 'singing', 'melody', 'rhythm', 'tune', 'sound', 'radio', 'listen', 'hear', 'loud', 'stereo', 'dance'],
    'fire': ['fire', 'burn', 'burning', 'flame', 'flames', 'hot', 'heat', 'blaze', 'fever', 'smoke', 'wild', 'ignite', 'ashes'],
    'star': ['star', 'stars', 'night', 'dream', 'dreams', 'shine', 'shining', 'glow', 'glowing', 'magic', 'spark', 'light', 'bright', 'wish', 'glitter'],
    'rain': ['rain', 'raining', 'cry', 'crying', 'tears', 'sad', 'storm', 'wash', 'wet', 'pour', 'drip', 'teardrop', 'teardrops', 'water', 'flood'],
    'wings': ['fly', 'flying', 'wings', 'bird', 'birds', 'angel', 'soar', 'feather', 'free', 'freedom'],
    'butterfly': ['butterfly', 'butterflies', 'flutter', 'gentle', 'soft', 'float', 'colors', 'colour'],
    'sun': ['sun', 'sunny', 'sunrise', 'sunset', 'daylight', 'morning', 'summer', 'gold', 'golden', 'warm', 'warmth'],
    'moon': ['moon', 'midnight', 'sleep', 'nighttime', 'dark', 'luna', 'twilight'],
    'lightning': ['lightning', 'electric', 'shock', 'strike', 'thunder', 'power', 'flash', 'energy', 'zap', 'voltage'],
    'eye': ['eye', 'eyes', 'look', 'looking', 'see', 'seeing', 'watch', 'stare', 'gaze', 'blind', 'sight', 'view'],
    'clock': ['time', 'clock', 'hours', 'late', 'seconds', 'minutes', 'years', 'forever', 'tick', 'ticking', 'wait', 'waiting'],
    'coffee': ['coffee', 'tea', 'cup', 'morning', 'wake', 'drink', 'warm', 'cafe', 'mug'],
    'ghost': ['ghost', 'ghosts', 'haunt', 'haunted', 'dead', 'spooky', 'fade', 'fading', 'disappear', 'past', 'vanish'],
    'crown': ['crown', 'king', 'queen', 'prince', 'princess', 'royal', 'royalty', 'rule', 'ruling', 'boss', 'champ', 'throne'],
    'diamond': ['diamond', 'diamonds', 'gem', 'gems', 'rich', 'wealth', 'jewel', 'precious', 'crystal'],
    'car': ['car', 'drive', 'driving', 'ride', 'riding', 'road', 'street', 'highway', 'speed', 'wheels', 'fast', 'traffic'],
    'key': ['key', 'keys', 'lock', 'secret', 'open', 'unlock', 'close', 'gate'],
    'sword': ['sword', 'knife', 'dagger', 'cut', 'blade', 'sharp', 'fight', 'war', 'wound', 'stab'],
    'bulb': ['idea', 'know', 'learn', 'glow', 'bulb', 'clever', 'genius', 'truth', 'mind', 'brain'],
    'target': ['target', 'aim', 'shoot', 'hit', 'bullseye', 'goal', 'score'],
    'wave': ['wave', 'waves', 'ocean', 'sea', 'water', 'swim', 'swimming', 'tide', 'deep', 'drift', 'drifting', 'river', 'boat', 'pulse', 'heartbeat'],
    'anchor': ['anchor', 'hold', 'sink', 'heavy', 'harbor', 'weight', 'ground'],
    'dice': ['dice', 'game', 'play', 'gamble', 'luck', 'chance', 'roll', 'bet', 'win', 'lose'],
    'planet': ['planet', 'space', 'universe', 'earth', 'galaxy', 'orbit', 'alien', 'rocket'],
    'leaf': ['leaf', 'leaves', 'autumn', 'fall', 'falling', 'tree', 'wind', 'nature', 'breeze'],
    'candle': ['candle', 'wick', 'wax', 'darkness', 'pray', 'prayer'],
    'phone': ['phone', 'call', 'calling', 'text', 'message', 'ring', 'ringing', 'hello', 'dial'],
    'pill': ['pill', 'pills', 'medicine', 'drug', 'drugs', 'high', 'numb', 'cure', 'heal', 'disease', 'sick'],
    'glasses': ['glasses', 'shades', 'cool', 'style', 'fashion', 'sunglasses'],
    'skull': ['skull', 'skeleton', 'poison', 'toxic', 'danger', 'die', 'dying', 'death', 'grave'],
    'balloon': ['balloon', 'balloons', 'party', 'birthday', 'celebrate'],
    'bell': ['bell', 'bells', 'chime', 'church', 'alarm'],
    'gift': ['gift', 'present', 'give', 'giving', 'surprise', 'wrap', 'package'],
    'shoe': ['shoes', 'shoe', 'sneaker', 'sneakers', 'keds', 'socks', 'walk', 'step', 'feet'],
    'wine': ['wine', 'glass', 'drink', 'drinking', 'alcohol', 'bar', 'bottle', 'cheers', 'toast'],
    'headphones': ['headphones', 'earphones', 'bass', 'audio'],
    'battery': ['battery', 'charge', 'empty', 'full', 'energy'],
    'bow': ['cupid', 'bow', 'archery'],
    'cloud': ['cloud', 'clouds', 'fog', 'gray', 'grey', 'fluffy', 'weather'],
    'cherry': ['cherry', 'cherries', 'fruit', 'taste', 'berry', 'pie'],
    'padlock': ['locked', 'safe', 'secure', 'protect', 'guard'],
    'bubble': ['think', 'thinking', 'thought', 'thoughts', 'talk', 'talking', 'say', 'saying', 'speak', 'words', 'tell', 'whisper'],
    'box': ['name', 'title', 'brand', 'label', 'tag', 'number', 'one', 'best', 'legend'],
    'circle': ['all', 'everything', 'world', 'forever', 'always', 'one', 'whole', 'complete', 'around'],
    'arrow': ['you', 'me', 'her', 'him', 'them', 'there', 'here', 'point', 'straight', 'direct']
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
        self.last_doodle = ""
        self.last_composition = ""

    def reset_song(self, song_title, artist=""):
        self.chorus_counts = {}
        self.song_history = []
        self.last_doodle = ""
        self.last_composition = ""
        
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
                "fx_flags": 0,
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

        # 2. Strict Semantic Doodle Detection (NO RANDOM ARBITRARY ICONS!)
        detected_doodle = "NONE"
        for doodle_type, triggers in DOODLE_MAP.items():
            for w in words:
                if w.lower() in triggers:
                    cand = doodle_type.upper()
                    if cand != self.last_doodle or len(words) <= 2:
                        detected_doodle = cand
                        break
            if detected_doodle != "NONE":
                break

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

        # Strict 128px Screen Width Word Balancing
        if len(prefix_str) > 18:
            pw = prefix_str.split()
            prefix_str = " ".join(pw[-2:]) if len(pw) >= 2 else prefix_str[:18]

        if len(suffix_str) > 18:
            sw = suffix_str.split()
            suffix_str = " ".join(sw[:2]) if len(sw) >= 2 else suffix_str[:18]

        # 4. Rich Compositional Archetypes (8 Archetypes!)
        compositions = ["CENTER", "DIAGONAL", "STACKED", "MONOLITH", "INVERSE", "COMIC", "SPLIT", "DREAMY"]
        
        # Smart assignment based on line characteristics and 128x48 resolution
        if len(words) <= 2 and len(clean_text) <= 12:
            composition = random.choice(["MONOLITH", "INVERSE", "CENTER"])
        elif len(focal_word) > 8:
            composition = random.choice(["STACKED", "CENTER", "INVERSE"])
        elif detected_metaphor in ["FALLING", "FLYING", "RUNNING"]:
            composition = random.choice(["DIAGONAL", "SPLIT", "DREAMY"])
        elif detected_doodle in ["BUBBLE", "NOTE"]:
            composition = "COMIC"
        elif len(clean_text) > 24:
            composition = random.choice(["STACKED", "DIAGONAL", "CENTER"])
        else:
            cand = [c for c in compositions if c != self.last_composition]
            composition = random.choice(cand)

        self.last_composition = composition

        # 5. Dynamic Visual FX Flags (Bitmask: 1=InverseBadge, 2=CornerFrames, 4=LeftAccentBar)
        fx_flags = 0
        if composition == "INVERSE" or (random.random() < 0.16 and len(focal_word) > 2):
            fx_flags |= 1 # Inverse Badge
        if composition == "MONOLITH" or random.random() < 0.12:
            fx_flags |= 2 # Corner Frames
        if composition == "STACKED":
            fx_flags |= 4 # Left Accent Bar

        # 6. Dynamic Tilt Angle & Underline
        tilt_angle = 0
        if composition not in ["MONOLITH", "STACKED"]:
            tilt_angle = random.choice([-4, -2, 0, 2, 4])
            
        has_underline = (detected_doodle == "UNDERLINE" or (random.random() < 0.15 and detected_doodle in ["NONE", "NOTE"])) and len(focal_word) > 2

        # 7. Deep Font Preset Pairing (Presets 0 to 7 covering 16 distinct font combinations!)
        font_preset = random.randint(0, 7)

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
            "fx_flags": fx_flags,
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
            f"{scene['duration_ms']}|{scene.get('font_preset', 0)}|{scene.get('fx_flags', 0)}\n"
        )
