"""
LyricCast Context-Aware Semantic Emoji Engine (Expressive++)
Vetted 1-Bit Monochrome Emojis, Kinetic Motion Modes & Dynamic Repetition Cycling.
"""

import re

# Motion Type Constants
MOTION_FLOAT = 0       # Gentle sinusoidal floating/bobbing (dreamy, mellow, celestial)
MOTION_HEARTBEAT = 1   # Authentic lub-dub double-beat scale pulse (hearts, romance, chest)
MOTION_FIRE = 2        # Chaotic high-frequency jitter/flicker (fire, lightning, hype, energy)
MOTION_DANCE = 3       # Rhythmic horizontal & vertical rock/sway (party, dancing, instruments)
MOTION_POP = 4         # Elastic pop entrance (footwear, rockets, explosions, pops)
MOTION_EYES_SCAN = 5   # Horizontal watchful scanning side-to-side (eyes, glance, gaze)

EMOJI_MOTION_MAP = {
    "❤️": MOTION_HEARTBEAT,
    "❤": MOTION_HEARTBEAT,
    "💓": MOTION_HEARTBEAT,
    "💔": MOTION_HEARTBEAT,
    "💘": MOTION_HEARTBEAT,
    "💖": MOTION_HEARTBEAT,
    "💞": MOTION_HEARTBEAT,
    "🔥": MOTION_FIRE,
    "⚡": MOTION_FIRE,
    "💥": MOTION_FIRE,
    "🪩": MOTION_DANCE,
    "💃": MOTION_DANCE,
    "🕺": MOTION_DANCE,
    "🎸": MOTION_DANCE,
    "🥁": MOTION_DANCE,
    "🎷": MOTION_DANCE,
    "🎺": MOTION_DANCE,
    "🎹": MOTION_DANCE,
    "👟": MOTION_POP,
    "🚀": MOTION_POP,
    "💣": MOTION_POP,
    "🎉": MOTION_POP,
    "🍾": MOTION_POP,
    "👀": MOTION_EYES_SCAN,
    "👁️": MOTION_EYES_SCAN,
}

def get_motion_for_emoji(emoji):
    return EMOJI_MOTION_MAP.get(emoji, MOTION_FLOAT)

# Dynamic Repetition Variations for Common Song Choruses & Refrains
# Guarantees repeated lines feel fresh, vibrant, and never stale!
REPETITION_VARIATIONS = {
    "shut up and dance": [
        [("🪩", MOTION_DANCE), ("💃", MOTION_DANCE)],
        [("💃", MOTION_DANCE), ("⚡", MOTION_FIRE)],
        [("🪩", MOTION_DANCE), ("✨", MOTION_FLOAT)],
        [("💃", MOTION_DANCE), ("❤️", MOTION_HEARTBEAT)],
    ],
    "dance with me": [
        [("💃", MOTION_DANCE), ("🪩", MOTION_DANCE)],
        [("💃", MOTION_DANCE), ("⚡", MOTION_FIRE)],
        [("💃", MOTION_DANCE), ("✨", MOTION_FLOAT)],
        [("💃", MOTION_DANCE), ("❤️", MOTION_HEARTBEAT)],
    ],
    "don't you dare look back": [
        [("✋", MOTION_FLOAT), ("👀", MOTION_FLOAT)],
        [("👀", MOTION_FLOAT), ("⚡", MOTION_FIRE)],
        [("✋", MOTION_FLOAT), ("✨", MOTION_FLOAT)],
    ],
    "keep your eyes on me": [
        [("👀", MOTION_FLOAT), ("✨", MOTION_FLOAT)],
        [("👀", MOTION_FLOAT), ("❤️", MOTION_HEARTBEAT)],
        [("👀", MOTION_FLOAT), ("⚡", MOTION_FIRE)],
    ],
    "bound to get together": [
        [("❤️", MOTION_HEARTBEAT), ("✨", MOTION_FLOAT)],
        [("💞", MOTION_HEARTBEAT), ("⭐", MOTION_FLOAT)],
        [("❤️", MOTION_HEARTBEAT), ("⚡", MOTION_FIRE)],
    ],
    "this woman is my destiny": [
        [("👑", MOTION_FLOAT), ("🔮", MOTION_FLOAT)],
        [("💃", MOTION_DANCE), ("✨", MOTION_FLOAT)],
        [("❤️", MOTION_HEARTBEAT), ("🔮", MOTION_FLOAT)],
    ],
    "felt it in my chest": [
        [("❤️", MOTION_HEARTBEAT), ("⚡", MOTION_FIRE)],
        [("❤️", MOTION_HEARTBEAT), ("✨", MOTION_FLOAT)],
    ],
    "backless dress": [
        [("👗", MOTION_FLOAT), ("👟", MOTION_POP)],
        [("👗", MOTION_FLOAT), ("✨", MOTION_FLOAT)],
    ],
    "beat up sneaks": [
        [("👟", MOTION_POP), ("👗", MOTION_FLOAT)],
        [("👟", MOTION_POP), ("⚡", MOTION_FIRE)],
    ],
}

LINE_REPETITION_TRACKER = {}

def reset_repetition_tracker():
    global LINE_REPETITION_TRACKER
    LINE_REPETITION_TRACKER.clear()

# Layer 1: High-Priority Multi-Word Lyrical Idioms & Phrases
# Strictly vetted 1-bit monochrome icons: crisp, high contrast silhouettes only!
LYRIC_IDIOMS = [
    # Dancing, Floor & Party
    ("shut up and dance", "🪩"),
    ("dance with me", "💃"),
    ("dancing with me", "💃"),
    ("dancing in the dark", "🕺"),
    ("dance in the dark", "🕺"),
    ("took the floor", "🪩"),
    ("take the floor", "🪩"),
    ("on the floor", "🪩"),
    ("dance floor", "🪩"),
    ("party all night", "🎉"),
    ("all night long", "🪩"),
    ("turn the music up", "🎵"),
    ("turn it up", "⚡"),
    ("pop the champagne", "🍾"),
    ("champagne problems", "🍾"),
    ("hands in the air", "✋"),
    ("good vibes", "✨"),
    ("wild side", "⚡"),

    # Looking, Eyes & Staring
    ("eyes on me", "👀"),
    ("eyes on you", "👀"),
    ("look back", "👀"),
    ("looking back", "👀"),
    ("look at me", "👀"),
    ("look at you", "👀"),
    ("in your eyes", "👀"),
    ("in my eyes", "👀"),
    ("staring at", "👀"),
    ("watching me", "👀"),
    ("watching you", "👀"),
    ("see the light", "💡"),

    # Holding, Hands & Arms (Replaced 🛑 with ✋ for pristine 1-bit contrast)
    ("holding back", "✋"),
    ("hold back", "✋"),
    ("hold my hand", "🤝"),
    ("take my hand", "🤝"),
    ("took my arm", "🤝"),
    ("take my arm", "🤝"),
    ("in your arms", "🤝"),
    ("in my arms", "🤝"),
    ("hold me tight", "🤝"),
    ("hold on", "✋"),

    # Romance, Love & Destiny (Replaced complex ZWJ emojis with bold ❤️)
    ("fall in love", "💘"),
    ("falling in love", "💘"),
    ("in love with", "❤️"),
    ("make love", "💋"),
    ("kiss me", "💋"),
    ("kiss you", "💋"),
    ("take my breath away", "🌹"),
    ("head over heels", "❤️"),
    ("die for you", "🥀"),
    ("heart of gold", "❤️"),
    ("bound to get together", "❤️"),
    ("bound to be together", "❤️"),
    ("get together", "✨"),
    ("be together", "❤️"),
    ("my destiny", "🔮"),
    ("you and me", "❤️"),
    ("you and i", "❤️"),
    ("stay with me", "❤️"),
    ("teenage dream", "💭"),

    # Heartbreak & Sadness
    ("broken heart", "💔"),
    ("break my heart", "💔"),
    ("breaks my heart", "💔"),
    ("broke my heart", "💔"),
    ("cry me a river", "🌧️"),
    ("tears fall", "🌧️"),
    ("tears falling", "🌧️"),
    ("tears in my eyes", "🌧️"),
    ("tears rolling down", "🌧️"),
    ("heart of stone", "💔"),
    ("cold as ice", "❄️"),
    ("left me alone", "🥀"),
    ("miss you so", "❤️"),
    ("felt it in my chest", "❤️"),
    ("in my chest", "❤️"),

    # Energy, Fire & Speed
    ("on fire", "🔥"),
    ("set fire to", "🔥"),
    ("catch on fire", "🔥"),
    ("burn it down", "🔥"),
    ("light it up", "⚡"),
    ("light up the", "✨"),
    ("blow my mind", "💥"),
    ("rock and roll", "🎸"),
    ("rock n roll", "🎸"),
    ("bad guy", "⚡"),
    ("bad girl", "⚡"),
    ("ride or die", "🏎️"),
    ("bullet proof", "🛡️"),
    ("here we go", "🚀"),
    ("let us go", "🚀"),
    ("let's go", "🚀"),
    ("beat up sneaks", "👟"),
    ("beat up sneakers", "👟"),
    ("backless dress", "👗"),
    ("faded light", "🕯️"),

    # Dreamy, Space & Night
    ("head in the clouds", "☁️"),
    ("under the stars", "🌌"),
    ("in the clouds", "☁️"),
    ("fly away", "🕊️"),
    ("victims of the night", "🌙"),
    ("middle of the night", "🌙"),
    ("in the night", "🌙"),
    ("late night", "🌙"),
    ("sweet dreams", "🌙"),
    ("golden hour", "☀️"),
    ("lost in the dark", "🕯️"),
    ("shine bright", "🌟"),

    # Mind, Communication & Questions (Replaced 🤷 with ❓)
    ("in my head", "💭"),
    ("in my mind", "💭"),
    ("don't know", "❓"),
    ("dont know", "❓"),
    ("ring my bell", "🔔"),
    ("call my phone", "📱"),
    ("call me", "📱"),
    ("run away", "🏃"),
    ("running away", "🏃"),
    ("out of time", "⏳"),
    ("time flies", "⏳"),
    ("drowning in", "🌊"),
]

# Layer 2: Comprehensive Lyrical Semantic Lexicon (Word Stems -> Vetted Emoji)
SEMANTIC_LEXICON = [
    # 1. Vision, Eyes & Gaze
    ("👀", ["eye", "eyes", "look", "looks", "looked", "looking", "stare", "staring", "gaze", "see", "saw", "seeing", "sight", "watch", "watching", "view", "glance"]),

    # 2. Hands, Arms, Touch & Holding
    ("🤝", ["hand", "hands", "hold", "holds", "holding", "held", "touch", "touching", "touched", "reach", "reaching", "grab", "grabbed", "arm", "arms", "closer"]),
    ("✋", ["stop", "stopped", "wait", "waiting", "pause", "enough", "stay", "halt", "dare"]),

    # 3. People, Women, Men & Identity
    ("💃", ["woman", "lady", "female", "girl", "girls", "juliet"]),
    ("🕺", ["man", "guy", "boy", "boys", "gentleman", "dude", "romeo"]),
    ("👑", ["queen", "king", "prince", "princess", "royal", "royalty", "reign", "monarch", "throne", "goddess"]),

    # 4. Clothing, Style & Footwear
    ("👗", ["dress", "skirt", "gown", "outfit", "wear", "wearing", "backless", "fashion"]),
    ("👟", ["sneaks", "sneaker", "sneakers", "shoe", "shoes", "boots", "heels", "kicks", "foot", "feet", "step", "steps"]),

    # 5. Chest, Heart & Physical Sensations
    ("❤️", ["chest", "heart", "heartbeat", "pulse", "breath", "breathe", "breathing", "alive", "flutter", "pounding", "love", "loved", "loving", "lover", "darling", "sweetheart", "romance", "romantic", "adore", "beloved", "together"]),
    ("💘", ["cupid", "arrow", "smitten", "lovesick", "fallen"]),
    ("💋", ["kiss", "kisses", "kissing", "kissed", "lips", "lipstick", "smooch"]),
    ("🌹", ["rose", "roses", "petal", "petals", "bouquet", "flower", "flowers", "bloom"]),
    ("💌", ["letter", "letters", "postcard", "note", "envelope", "written"]),
    ("💍", ["marry", "wedding", "proposal", "engaged", "bride", "groom", "vows"]),

    # 6. Heartbreak, Sadness, Crying & Scars
    ("💔", ["heartbreak", "heartbroken", "apart", "broken", "shattered", "break", "broke"]),
    ("🥀", ["wither", "wilted", "faded", "dying", "decay", "forsaken", "abandoned"]),
    ("🌧️", ["cry", "crying", "cried", "tears", "weep", "weeping", "sob", "sobbing", "rain", "raining", "rainy", "storm", "pour", "pouring", "wet", "puddle", "drizzle"]),
    ("❄️", ["lonely", "alone", "cold", "empty", "emptiness", "sorrow", "regret", "snow", "ice", "icy", "frozen", "freeze", "chilly", "frost"]),

    # 7. Nightlife, Dancing & Party
    ("🪩", ["disco", "party", "club", "groove", "groovin", "floor", "mirrors", "boogie", "discothque", "discotheque"]),
    ("💃", ["dance", "dancing", "danced", "dancer", "dancers", "waltz", "salsa"]),
    ("🎉", ["celebrate", "celebration", "confetti", "cheer", "cheers", "jubilee"]),
    ("🍾", ["champagne", "popping", "bottle", "bottles", "cork", "sparkling"]),
    ("🥂", ["toast", "glasses", "cheers", "clink", "drink", "drinks", "drunk", "wasted", "tipsy"]),

    # 8. Music, Instruments & Audio
    ("🎸", ["guitar", "riff", "chords", "strum", "strumming", "bass", "fender", "gibson", "acoustic", "electric"]),
    ("🎹", ["piano", "keys", "keyboard", "melody", "synth", "synthesizer"]),
    ("🥁", ["drum", "drums", "drummer", "snare", "cymbal", "percussion"]),
    ("🎷", ["sax", "saxophone", "jazz", "brass"]),
    ("🎺", ["trumpet", "horn", "fanfare"]),
    ("🎤", ["sing", "singing", "sang", "singer", "vocal", "vocals", "mic", "microphone", "rap", "rapping", "rapper", "verse", "chorus"]),
    ("🔔", ["bell", "bells", "chime", "chimes", "alarm", "ring"]),

    # 9. Energy, Fire, Lightning & Power
    ("🔥", ["fire", "flame", "flames", "burn", "burning", "burned", "blaze", "blazing", "hot", "heat", "ignite"]),
    ("⚡", ["lightning", "electric", "electricity", "thunder", "shock", "voltage", "power", "flash", "spark", "energy", "physical", "chemical", "kryptonite"]),
    ("💎", ["diamond", "diamonds", "gem", "gems", "jewel", "jewels", "jewelry", "crystal", "precious"]),
    ("🚀", ["rocket", "blast", "launch", "soar", "orbit", "liftoff"]),
    ("💣", ["bomb", "dynamite", "explode", "explosion", "boom"]),

    # 10. Night, Space, Mystery & Cosmic
    ("🌙", ["moon", "luna", "night", "nighttime", "midnight", "crescent", "nocturnal"]),
    ("⭐", ["star", "stars", "starlight", "constellation", "shine", "shining", "glow", "glowing", "wish"]),
    ("🌌", ["galaxy", "milky", "universe", "cosmos", "cosmic", "infinity"]),
    ("🪐", ["planet", "planets", "saturn", "jupiter", "mars", "space"]),
    ("☁️", ["cloud", "clouds", "cloudy", "haze", "mist", "fog"]),
    ("✨", ["sparkle", "sparkles", "magic", "magical", "glimmer", "shimmer", "radiant", "dazzle", "bound", "wonder"]),
    ("🕯️", ["candle", "wick", "wax", "lantern", "flame", "light"]),
    ("💭", ["think", "thinking", "thought", "thoughts", "mind", "wonder", "wondering", "dream", "dreaming", "dreams", "remember", "head", "heavy"]),
    ("🔮", ["destiny", "fate", "fortune", "future", "prophecy"]),
    ("❓", ["know", "happened", "question", "how", "why"]),

    # 11. Nature, Flora, Weather & Geography
    ("🌲", ["tree", "trees", "pine", "forest", "woods", "timber", "evergreen", "spruce"]),
    ("🌳", ["oak", "branches", "grove", "canopy", "leaves"]),
    ("🌴", ["palm", "palms", "tropical", "island", "coconut"]),
    ("🌱", ["grow", "growing", "grown", "sprout", "plant", "plants", "roots", "seed", "green"]),
    ("🌍", ["earth", "world", "globe", "planet", "continents", "worldwide", "global"]),
    ("🗺️", ["map", "country", "countries", "border", "borders", "land", "miles", "journey", "compass", "navigation"]),
    ("🏔️", ["mountain", "mountains", "peak", "summit", "climb", "climbing", "rocky", "hills", "valley", "canyon"]),
    ("🏙️", ["city", "cities", "town", "skyline", "buildings", "downtown", "streets", "york", "tokyo", "paris", "london"]),
    ("🌺", ["flower", "flowers", "cherry", "blossom", "blossoms", "bloom", "blooming"]),
    ("🍁", ["autumn", "fall", "leaf", "foliage"]),
    ("🦋", ["butterfly", "butterflies", "cocoon"]),
    ("🌊", ["wave", "waves", "ocean", "sea", "tide", "tides", "surf", "drown", "deep", "river", "shore", "beach", "water"]),
    ("☀️", ["sun", "sunny", "sunshine", "daylight", "bright", "golden", "summer", "warmth", "dawn", "sunrise"]),
    ("🌅", ["sunset", "dusk", "evening", "horizon"]),

    # 12. Speed, Driving, Travel & Escape
    ("🏎️", ["race", "racing", "speed", "fast", "ferrari", "porsche", "drift", "engine"]),
    ("🚗", ["car", "cars", "drive", "driving", "drove", "ride", "riding", "wheels", "traffic", "road", "street", "highway"]),
    ("🏃", ["run", "running", "ran", "sprint", "chase", "chasing", "flee", "escape"]),

    # 13. Street, Danger & Edge
    ("💀", ["dead", "death", "die", "dying", "died", "skull", "skeleton", "morbid"]),
    ("🛡️", ["shield", "armor", "guard", "protect", "safe"]),
    ("🕊️", ["peace", "dove", "free", "freedom", "bird", "wings"]),
]

KEYWORD_TO_EMOJI = {}
for emoji, keywords in SEMANTIC_LEXICON:
    for kw in keywords:
        KEYWORD_TO_EMOJI[kw.lower()] = emoji

DISAMBIGUATION_BLACKLIST = {
    "fast": "🍔",
    "play": "🎮",
    "class": "🏀",
    "board": "🛹",
    "rip": "⚰️",
    "blind": "🕶️",
    "bag": "💰",
}

# High-contrast 1-bit vetted fallback palette (NO generic music notes - music only plays when singing/instruments mentioned!)
FALLBACK_PALETTE = ["✨", "🌟", "💫", "💭", "⭐", "❤️", "🌙", "🕊️"]

def analyze_lyric_multi_emoji(line_text, focal_word=""):
    """
    Extracts up to 2 distinct, highly contextual vetted monochrome emojis
    with dynamic repetition cycling.
    Returns:
        List of tuples: [(emoji1, motion1), (emoji2, motion2)] or [(emoji1, motion1)]
    """
    if not line_text or not line_text.strip():
        return [("✨", MOTION_FLOAT)]

    clean_text = line_text.lower().strip()
    words = [re.sub(r'[^\w]', '', w).lower() for w in clean_text.split() if w]

    # 1. Repetition Cycling for Refrains / Choruses
    for phrase, variations in REPETITION_VARIATIONS.items():
        if phrase in clean_text:
            idx = LINE_REPETITION_TRACKER.get(phrase, 0)
            LINE_REPETITION_TRACKER[phrase] = idx + 1
            return variations[idx % len(variations)]

    detected_emojis = []

    # 2. Multi-word Idioms & Phrases
    remainder_text = clean_text
    for idiom, emoji in LYRIC_IDIOMS:
        if idiom in remainder_text:
            m = get_motion_for_emoji(emoji)
            if (emoji, m) not in detected_emojis:
                detected_emojis.append((emoji, m))
                remainder_text = remainder_text.replace(idiom, " ")
            if len(detected_emojis) >= 2:
                return detected_emojis

    # 3. Focal Word Analysis
    if focal_word:
        clean_focal = re.sub(r'[^\w]', '', focal_word).lower()
        if clean_focal in KEYWORD_TO_EMOJI:
            em = KEYWORD_TO_EMOJI[clean_focal]
            m = get_motion_for_emoji(em)
            if (em, m) not in detected_emojis:
                detected_emojis.append((em, m))
            if len(detected_emojis) >= 2:
                return detected_emojis

    # 4. Secondary Keyword Scan on Remaining Words
    rem_words = [re.sub(r'[^\w]', '', w).lower() for w in remainder_text.split() if w]
    for w in rem_words:
        if w in KEYWORD_TO_EMOJI:
            em = KEYWORD_TO_EMOJI[w]
            if w in DISAMBIGUATION_BLACKLIST and DISAMBIGUATION_BLACKLIST[w] == em:
                continue
            m = get_motion_for_emoji(em)
            if (em, m) not in detected_emojis:
                detected_emojis.append((em, m))
            if len(detected_emojis) >= 2:
                return detected_emojis

    # 5. Single match found -> Check if we can pair with an evocative secondary
    if len(detected_emojis) == 1:
        # If primary is heart or dance, we can leave as 1 hero or pair with sparkle
        return detected_emojis

    # 6. Sentiment / Valence Fallback
    joy_stems = {"good", "great", "better", "alive", "free", "beautiful", "high", "bright", "together", "smile", "happy", "sun"}
    sad_stems = {"never", "nowhere", "hard", "dark", "heavy", "trouble", "tired", "fall", "down", "lost", "nobody", "wrong", "sorry"}
    love_stems = {"baby", "babe", "girl", "boy", "forever", "sweet", "touch", "feel", "closer", "stay", "us", "mine"}
    hype_stems = {"stop", "go", "ready", "now", "jump", "bang", "shout", "make", "take", "bring", "push", "move"}

    word_set = set(words)
    if word_set & joy_stems:
        return [("✨", MOTION_FLOAT)]
    if word_set & sad_stems:
        return [("🌧️", MOTION_FLOAT)]
    if word_set & love_stems:
        return [("❤️", MOTION_HEARTBEAT)]
    if word_set & hype_stems:
        return [("⚡", MOTION_FIRE)]

    # 7. Non-Repeating Dynamic Fallback Palette
    line_hash = abs(hash(clean_text))
    em = FALLBACK_PALETTE[line_hash % len(FALLBACK_PALETTE)]
    return [(em, get_motion_for_emoji(em))]

def analyze_lyric_semantics(line_text, focal_word=""):
    """
    Backward-compatible single emoji extractor.
    """
    multi = analyze_lyric_multi_emoji(line_text, focal_word=focal_word)
    if multi:
        return multi[0]
    return "✨", MOTION_FLOAT
