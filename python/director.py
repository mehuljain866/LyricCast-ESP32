"""
LyricCast Semantic Director & Storyboard Generator (V8 - Zero Word Truncation & Precise Lyrics Delivery)
Analyzes lyric lines, assigns typographic hierarchy across 16 font styles,
50+ context-aware procedural vector doodles, 8 compositional archetypes,
and strictly PRESERVES 100% of all sung words (NO skipping, NO truncation).
"""

import re
import random
import hashlib

METAPHORS = {
    'falling': ['fall', 'falling', 'fell', 'drop', 'dropping', 'down', 'sink', 'sinking', 'drown', 'drowning', 'bottom', 'low', 'deep'],
    'flying': ['fly', 'flying', 'flew', 'float', 'floating', 'sky', 'clouds', 'high', 'rise', 'rising', 'up', 'heaven', 'space', 'soar', 'wings', 'angel', 'higher', 'above'],
    'spinning': ['spin', 'spinning', 'rotate', 'round', 'around', 'circle', 'spiral', 'twist', 'whirl', 'roll', 'dance', 'dancing', 'dizzy'],
    'running': ['run', 'running', 'ran', 'fast', 'speed', 'chase', 'chasing', 'escape', 'away', 'drive', 'driving', 'hurry', 'rush', 'race', 'rushing'],
    'broken': ['break', 'broken', 'breaking', 'tear', 'tearing', 'apart', 'shatter', 'crush', 'crack', 'pieces', 'split', 'bleed', 'hurt', 'pain', 'scars'],
    'alone': ['alone', 'lonely', 'nobody', 'empty', 'silence', 'quiet', 'hollow', 'isolated', 'lost', 'darkness', 'dark', 'ghost', 'miss'],
    'scream': ['scream', 'screaming', 'shout', 'shouting', 'loud', 'yell', 'noise', 'roar', 'wild', 'crazy', 'mad', 'shook', 'boom'],
    'stay': ['stay', 'hold', 'never', 'forever', 'always', 'stop', 'stand', 'freeze', 'wait', 'waiting', 'keep'],
    'shake': ['shake', 'shaking', 'tremble', 'vibrate', 'quake', 'nervous', 'scared', 'fear', 'beat', 'pump']
}

# 50+ Semantic Doodle Keyword Categories with Context-Aware Directionals
DOODLE_MAP = {
    'down': ['down', 'bottom', 'low', 'drop', 'dropping', 'sink', 'sinking', 'below', 'under', 'ground', 'floor'],
    'up': ['up', 'higher', 'rise', 'rising', 'top', 'above', 'elevate', 'soar'],
    'left': ['left', 'back', 'behind', 'past', 'yesterday', 'return'],
    'phone': ['phone', 'call', 'calling', 'text', 'message', 'ring', 'ringing', 'hello', 'dial', 'telephone', 'talk'],
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
    'wings': ['wings', 'bird', 'birds', 'angel', 'feather', 'free', 'freedom'],
    'butterfly': ['butterfly', 'butterflies', 'flutter', 'gentle', 'soft', 'colors', 'colour'],
    'sun': ['sun', 'sunny', 'sunrise', 'sunset', 'daylight', 'morning', 'summer', 'gold', 'golden', 'warm', 'warmth'],
    'moon': ['moon', 'midnight', 'sleep', 'nighttime', 'dark', 'luna', 'twilight'],
    'lightning': ['lightning', 'electric', 'shock', 'strike', 'thunder', 'power', 'flash', 'energy', 'zap', 'voltage'],
    'eye': ['eye', 'eyes', 'look', 'looking', 'see', 'seeing', 'watch', 'stare', 'gaze', 'blind', 'sight', 'view'],
    'clock': ['time', 'clock', 'hours', 'late', 'seconds', 'minutes', 'years', 'forever', 'tick', 'ticking', 'wait', 'waiting'],
    'coffee': ['coffee', 'tea', 'cup', 'wake', 'drink', 'warm', 'cafe', 'mug'],
    'ghost': ['ghost', 'ghosts', 'haunt', 'haunted', 'dead', 'spooky', 'fade', 'fading', 'disappear', 'vanish'],
    'crown': ['crown', 'king', 'queen', 'prince', 'princess', 'royal', 'royalty', 'rule', 'ruling', 'boss', 'champ', 'throne'],
    'diamond': ['diamond', 'diamonds', 'gem', 'gems', 'rich', 'wealth', 'jewel', 'precious', 'crystal'],
    'car': ['car', 'drive', 'driving', 'ride', 'riding', 'road', 'street', 'highway', 'speed', 'wheels', 'fast', 'traffic'],
    'key': ['key', 'keys', 'lock', 'secret', 'open', 'unlock', 'close', 'gate'],
    'sword': ['sword', 'knife', 'dagger', 'cut', 'blade', 'sharp', 'fight', 'war', 'wound', 'stab'],
    'bulb': ['idea', 'know', 'learn', 'glow', 'bulb', 'clever', 'genius', 'truth', 'mind', 'brain'],
    'target': ['target', 'aim', 'shoot', 'hit', 'bullseye', 'goal', 'score'],
    'wave': ['wave', 'waves', 'ocean', 'sea', 'water', 'swim', 'swimming', 'tide', 'deep', 'drift', 'drifting', 'river', 'boat', 'pulse', 'heartbeat'],
    'anchor': ['anchor', 'hold', 'sink', 'heavy', 'harbor', 'weight'],
    'dice': ['dice', 'game', 'play', 'gamble', 'luck', 'chance', 'roll', 'bet', 'win', 'lose'],
    'planet': ['planet', 'space', 'universe', 'earth', 'galaxy', 'orbit', 'alien', 'rocket'],
    'leaf': ['leaf', 'leaves', 'autumn', 'tree', 'wind', 'nature', 'breeze'],
    'candle': ['candle', 'wick', 'wax', 'pray', 'prayer'],
    'pill': ['pill', 'pills', 'medicine', 'drug', 'drugs', 'numb', 'cure', 'heal', 'disease', 'sick'],
    'glasses': ['glasses', 'shades', 'cool', 'style', 'fashion', 'sunglasses'],
    'cloud': ['cloud', 'clouds', 'fog', 'weather', 'sky', 'sweater', 'rainy', 'haze', 'overcast', 'mist', 'wind', 'breeze', 'stormy', 'grey', 'gray', 'fluffy', 'air', 'soar'],
    'cherry': ['cherry', 'cherries', 'fruit', 'taste', 'berry', 'pie'],
    'padlock': ['locked', 'safe', 'secure', 'protect', 'guard'],
    'coffin': ['died', 'die', 'dying', 'death', 'dead', 'grave', 'coffin', 'casket', 'bury', 'buried', 'rip', 'funeral', 'tomb', 'cemetery'],
    'skull': ['skull', 'skeleton', 'poison', 'toxic', 'danger', 'kill', 'killer', 'murder'],
    'balloon': ['balloon', 'balloons', 'party', 'birthday', 'celebrate'],
    'bell': ['bell', 'bells', 'chime', 'church', 'alarm'],
    'gift': ['gift', 'present', 'give', 'giving', 'surprise', 'wrap', 'package'],
    'shoe': ['shoes', 'shoe', 'sneaker', 'sneakers', 'keds', 'socks', 'walk', 'step', 'feet'],
    'wine': ['wine', 'glass', 'drink', 'drinking', 'alcohol', 'bar', 'bottle', 'cheers', 'toast'],
    'headphones': ['headphones', 'earphones', 'bass', 'audio'],
    'battery': ['battery', 'charge', 'empty', 'full', 'energy'],
    'bow': ['cupid', 'bow', 'archery'],
    'bubble': ['think', 'thinking', 'thought', 'thoughts', 'say', 'saying', 'speak', 'words', 'tell', 'whisper'],
    'box': ['name', 'title', 'brand', 'label', 'tag', 'number', 'one', 'best', 'legend'],
    'circle': ['everything', 'world', 'whole', 'complete', 'around', 'orbit'],
    'arrow': ['point', 'straight', 'direct', 'ahead', 'direction']
}

# Genre & Energy Profiles for Intelligent Typographic Styling
GENRES = {
    'ROCK': ['rock', 'punk', 'metal', 'grunge', 'nirvana', 'arctic monkeys', 'blink-182', 'greenday', 'linkin', 'radiohead', 'strokes', 'killers', 'foals', 'guitar', 'drums'],
    'HIPHOP': ['rap', 'hiphop', 'trap', 'drill', 'drake', 'kendrick', 'travis', 'kanye', 'carti', 'future', '21 savage', 'eminem', 'cole', 'flacko', 'asap', 'beat', 'flow'],
    'INDIE': ['indie', 'neighbourhood', 'malcolm todd', 'dominic fike', 'clairo', 'boy pablo', 'mac demarco', 'rex orange', 'phoebe', 'cigarettes after sex', 'beach house', 'wallows', 'steve lacy', 'omar apollo'],
    'POP': ['pop', 'sabrina carpenter', 'dua lipa', 'taylor swift', 'ariana grande', 'olivia rodrigo', 'charli xcx', 'billie eilish', 'chappell roan', 'katy perry', 'bruno mars'],
    'RB_SOUL': ['r&b', 'soul', 'adele', 'frank ocean', 'sza', 'daniel caesar', 'giveon', 'brent faiyaz', 'h.e.r.', 'sam smith', 'leon bridges'],
    'ELECTRONIC': ['electronic', 'edm', 'dance', 'synth', 'daft punk', 'avicii', 'calvin harris', 'skrillex', 'disclosure', 'odesza', 'flume', 'fred again']
}

STOP_WORDS = {
    'a', 'an', 'the', 'and', 'or', 'but', 'if', 'in', 'on', 'at', 'to', 'for', 'with', 'by',
    'about', 'as', 'into', 'like', 'through', 'after', 'over', 'between', 'out', 'against',
    'during', 'without', 'before', 'under', 'around', 'among', 'is', 'am', 'are', 'was', 'were',
    'be', 'been', 'being', 'have', 'has', 'had', 'do', 'does', 'did', 'that', 'this', 'it', 'its',
    'i', 'me', 'my', 'myself', 'we', 'our', 'ours', 'ourselves', 'you', 'your', 'yours', 'yourself',
    'he', 'him', 'his', 'himself', 'she', 'her', 'hers', 'herself', 'they', 'them', 'their', 'theirs',
    'what', 'which', 'who', 'whom', 'whose', 'when', 'where', 'why', 'how', 'all', 'any', 'both',
    'each', 'few', 'more', 'most', 'other', 'some', 'such', 'no', 'nor', 'not', 'only', 'own',
    'same', 'so', 'than', 'too', 'very', 'can', 'will', 'just', 'should', 'now', 'there', 'here',
    'ill', "i'll", 'youll', "you'll", 'theyll', "they'll", 'well', "we'll", 'im', "i'm", 'youre', "you're",
    'were', "we're", 'theyre', "they're", 'ive', "i've", 'youve', "you've", 'weve', "we've",
    'id', "i'd", 'youd', "you'd", 'hed', "he'd", 'shed', "she'd", 'wed', "we'd",
    'dont', "don't", 'wont', "won't", 'cant', "can't", 'isnt', "isn't", 'arent', "aren't",
    'wasnt', "wasn't", 'werent', "weren't", 'hasnt', "hasn't", 'havent', "haven't",
    'couldnt', "couldn't", 'shouldnt', "shouldn't", 'wouldnt', "wouldn't", 'gotta', 'gonna', 'wanna'
}

class LyricDirector:
    def __init__(self):
        self.chorus_counts = {}
        self.song_history = []
        self.current_seed = 42
        self.last_doodle = ""
        self.last_composition = ""
        self.song_genre = "INDIE"
        self.base_energy = 3 # 1 (mellow/ballad) to 5 (explosive/rock)
        self.genre_adaptive = True
        self.default_font_preset = 0 # 0=Classic Animated Cursive
        self.song_font_preset = 0

    def set_genre_adaptive(self, enabled: bool):
        self.genre_adaptive = enabled

    def set_default_font_preset(self, preset: int):
        self.default_font_preset = preset

    def reset_song(self, song_title, artist=""):
        self.chorus_counts = {}
        self.song_history = []
        self.last_doodle = ""
        self.last_composition = ""
        
        # Analyze track metadata to determine genre & energy profile
        track_meta = f"{song_title} {artist}".lower()
        detected_genre = "INDIE" # Default fallback
        for genre, keywords in GENRES.items():
            for kw in keywords:
                if kw in track_meta:
                    detected_genre = genre
                    break
            if detected_genre != "INDIE":
                break

        self.song_genre = detected_genre
        
        # Base energy calculation
        if detected_genre in ['ROCK', 'HIPHOP', 'ELECTRONIC']:
            self.base_energy = 4
        elif detected_genre in ['POP']:
            self.base_energy = 3
        elif detected_genre in ['RB_SOUL']:
            self.base_energy = 2
        else:
            self.base_energy = 3

        # Lock ONE unified typographic theme for this entire song!
        if self.genre_adaptive:
            if self.song_genre == 'ROCK':
                self.song_font_preset = 4 # Unified Bold Sans
            elif self.song_genre == 'HIPHOP':
                self.song_font_preset = 3 # Unified Monospace
            elif self.song_genre in ['POP', 'ELECTRONIC']:
                self.song_font_preset = 1 # Unified Modern Sans
            elif self.song_genre == 'RB_SOUL':
                self.song_font_preset = 2 # Unified Editorial Serif
            else: # INDIE / Alternative (Sweater Weather, Dominic Fike, Malcolm Todd)
                self.song_font_preset = 0 # Unified Cursive Script
        else:
            self.song_font_preset = self.default_font_preset

        seed_str = f"{song_title}-{artist}"
        hash_val = int(hashlib.md5(seed_str.encode('utf-8')).hexdigest()[:8], 16)
        self.current_seed = hash_val & 0xFFFFFFFF
        random.seed(self.current_seed)

    def analyze_line(self, line_text, duration=2.5, song_position=0.0):
        clean_text = line_text.strip()
        words = clean_text.split()
        
        if not words or clean_text in ["...", "♫", "♥", "★", "☺"]:
            return {
                "type": "idle",
                "text": clean_text,
                "focal_word": "",
                "prefix": "",
                "suffix": "",
                "metaphor": "IDLE",
                "doodle": "NONE",
                "composition": "CENTER",
                "font_preset": self.song_font_preset,
                "fx_flags": 0,
                "has_underline": False,
                "tilt_angle": 0,
                "duration_ms": int(duration * 1000)
            }

        # 1. Detect Motion Metaphor
        detected_metaphor = "NORMAL"
        for meta, triggers in METAPHORS.items():
            for w in words:
                clean_w = re.sub(r'[^\w]', '', w).lower()
                if clean_w in triggers:
                    detected_metaphor = meta.upper()
                    break
            if detected_metaphor != "NORMAL":
                break

        # 2. Strict Semantic Doodle Detection (Context-Aware Directionals)
        detected_doodle = "NONE"
        for doodle_type, triggers in DOODLE_MAP.items():
            for w in words:
                clean_w = re.sub(r'[^\w]', '', w).lower()
                if clean_w in triggers:
                    cand = doodle_type.upper()
                    if cand != self.last_doodle or len(words) <= 2:
                        detected_doodle = cand
                        break
            if detected_doodle != "NONE":
                break

        self.last_doodle = detected_doodle

        # 3. Typographic Hierarchy (Vivid Emotional & Content Focal Word Selection)
        focal_index = -1
        max_score = -9999.0

        for idx, w in enumerate(words):
            clean_w = re.sub(r'[^\w]', '', w).lower()
            if len(clean_w) == 0:
                continue
            
            is_stop = clean_w in STOP_WORDS
            
            if is_stop:
                score = 1.0 # Low baseline for stop words
            else:
                # Real content words get strong baseline proportional to length and vividness
                score = 15.0 + len(clean_w) * 2.5
            
            # Semantic Doodle Match (+35 for high-impact visual match!)
            if detected_doodle != "NONE" and clean_w in DOODLE_MAP.get(detected_doodle.lower(), []) and not is_stop:
                score += 35.0
            
            # Motion Metaphor Match (+25)
            if detected_metaphor != "NORMAL" and clean_w in METAPHORS.get(detected_metaphor.lower(), []) and not is_stop:
                score += 25.0
                
            # Proper Noun / Capitalized word (+15)
            if w.isupper() and len(clean_w) > 1:
                score += 15.0
            elif w[0].isupper() and idx > 0 and not is_stop:
                score += 12.0
            
            # Line length balancing (gentle guidance so extreme lines don't overflow)
            prefix_cand = " ".join(words[:idx])
            suffix_cand = " ".join(words[idx + 1:])
            if len(prefix_cand) > 22:
                score -= (len(prefix_cand) - 22) * 1.5
            if len(suffix_cand) > 22:
                score -= (len(suffix_cand) - 22) * 1.5
            
            # Subtle position centering preference (+4)
            center_ratio = 1.0 - (abs(idx - (len(words) - 1) / 2.0) / max(1, (len(words) - 1) / 2.0))
            score += center_ratio * 4.0

            if score > max_score:
                max_score = score
                focal_index = idx

        if focal_index == -1:
            focal_index = len(words) // 2

        focal_word = words[focal_index] if 0 <= focal_index < len(words) else ""
        prefix_words = words[:focal_index]
        suffix_words = words[focal_index + 1:] if focal_index + 1 < len(words) else []

        # PRESERVE 100% OF ALL SUNG WORDS (ZERO TRUNCATION!)
        prefix_str = " ".join(prefix_words).strip()
        suffix_str = " ".join(suffix_words).strip()

        # 4. Line Energy & Emotion Scoring
        line_energy = self.base_energy
        if any(w.isupper() and len(w) > 1 for w in words) or "!" in clean_text:
            line_energy += 2
        if detected_metaphor in ["RUNNING", "SCREAM", "SHAKE", "FIRE"]:
            line_energy += 1
        elif detected_metaphor in ["ALONE", "FALLING", "STAY"]:
            line_energy -= 1

        # 5. Rich Compositional Archetypes
        compositions = ["CENTER", "DIAGONAL", "STACKED", "INVERSE", "COMIC", "SPLIT", "DREAMY"]
        
        if len(words) == 1:
            composition = random.choice(["MONOLITH", "INVERSE", "CENTER"])
        elif len(focal_word) > 8:
            composition = random.choice(["STACKED", "CENTER", "INVERSE"])
        elif detected_metaphor in ["FALLING", "FLYING", "RUNNING"]:
            composition = random.choice(["DIAGONAL", "SPLIT", "DREAMY"])
        elif detected_doodle in ["BUBBLE", "NOTE"]:
            composition = "COMIC"
        elif len(clean_text) > 22:
            composition = random.choice(["STACKED", "DIAGONAL", "CENTER"])
        else:
            cand = [c for c in compositions if c != self.last_composition]
            composition = random.choice(cand)

        self.last_composition = composition

        # 6. Dynamic Visual FX Flags (Bitmask: 1=InverseBadge, 2=CornerFrames, 4=LeftAccentBar)
        fx_flags = 0
        if (composition == "INVERSE" or line_energy >= 4) and len(focal_word) > 2:
            fx_flags |= 1 # Inverse Badge for punchy words / high energy
        if composition == "MONOLITH" or (line_energy <= 2 and random.random() < 0.2):
            fx_flags |= 2 # Corner Frames for dramatic/monolithic moments
        if composition == "STACKED" or self.song_genre == 'HIPHOP':
            fx_flags |= 4 # Left Accent Bar

        # 7. Dynamic Tilt Angle & Underline
        tilt_angle = 0
        if composition not in ["MONOLITH", "STACKED"]:
            tilt_angle = random.choice([-3, 0, 3])
            
        has_underline = (detected_doodle == "UNDERLINE" or (random.random() < 0.18 and detected_doodle in ["NONE", "NOTE"])) and len(focal_word) > 2

        # 8. Consistent Locked Font Preset for Entire Song (Zero mid-song font jumps!)
        font_preset = self.song_font_preset

        # Responsive Snappy Timing
        line_duration_ms = max(800, int(duration * 1000))

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
            "duration_ms": line_duration_ms
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
