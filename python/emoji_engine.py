"""
LyricCast Context-Aware Semantic Emoji Engine (Expressive++)
Provides multi-layered lyrical intelligence:
1. High-priority lyrical idioms & multi-word phrase matching
2. Contextual keyword disambiguation (eliminating false positives like burgers for 'fast' or gamepads for 'play')
3. Lyrical sentiment & mood fallback
4. Kinetic motion choreography assignment (Bounce, Float, Pulse, Wiggle, Sparkle)
"""

import re

# Motion Types matching ESP32 firmware:
# 0: BOUNCE  (Energetic bobbing + elastic entrance pop)
# 1: FLOAT   (Dreamy sinusoidal drift & wave)
# 2: PULSE   (Heartbeat rhythmic double-throb)
# 3: WIGGLE  (High-energy rapid tremor/shiver)
# 4: SPARKLE (Subtle drift with orbiting micro-sparkles)
MOTION_BOUNCE = 0
MOTION_FLOAT = 1
MOTION_PULSE = 2
MOTION_WIGGLE = 3
MOTION_SPARKLE = 4

# Layer 1: High-Priority Multi-Word Lyrical Idioms & Phrases
# (Phrase -> (Emoji, MotionType))
LYRIC_IDIOMS = [
    # Dancing & Party
    ("shut up and dance", "🪩", MOTION_BOUNCE),
    ("dance with me", "💃", MOTION_BOUNCE),
    ("dancing with me", "💃", MOTION_BOUNCE),
    ("dancing in the dark", "🕺", MOTION_BOUNCE),
    ("dance in the dark", "🕺", MOTION_BOUNCE),
    ("party all night", "🎉", MOTION_BOUNCE),
    ("all night long", "🪩", MOTION_BOUNCE),
    ("turn the music up", "🔊", MOTION_BOUNCE),
    ("turn it up", "🔊", MOTION_BOUNCE),
    ("pop the champagne", "🍾", MOTION_BOUNCE),
    ("champagne problems", "🍾", MOTION_FLOAT),
    ("hands in the air", "🙌", MOTION_BOUNCE),
    ("good vibes", "✨", MOTION_SPARKLE),
    ("wild side", "🐺", MOTION_WIGGLE),
    
    # Love & Romance
    ("fall in love", "💘", MOTION_PULSE),
    ("falling in love", "💘", MOTION_PULSE),
    ("in love with", "❤️", MOTION_PULSE),
    ("make love", "💋", MOTION_PULSE),
    ("kiss me", "💋", MOTION_PULSE),
    ("kiss you", "💋", MOTION_PULSE),
    ("hold my hand", "🤝", MOTION_PULSE),
    ("take my hand", "🤝", MOTION_PULSE),
    ("take my breath away", "🌹", MOTION_FLOAT),
    ("head over heels", "🥰", MOTION_PULSE),
    ("die for you", "🥀", MOTION_FLOAT),
    ("heart of gold", "💛", MOTION_SPARKLE),
    ("you and me", "💑", MOTION_PULSE),
    ("stay with me", "🥺", MOTION_PULSE),
    
    # Heartbreak & Sadness
    ("broken heart", "💔", MOTION_FLOAT),
    ("break my heart", "💔", MOTION_FLOAT),
    ("breaks my heart", "💔", MOTION_FLOAT),
    ("broke my heart", "💔", MOTION_FLOAT),
    ("cry me a river", "😭", MOTION_FLOAT),
    ("tears fall", "😢", MOTION_FLOAT),
    ("tears falling", "😢", MOTION_FLOAT),
    ("tears in my eyes", "🥺", MOTION_FLOAT),
    ("tears rolling down", "😭", MOTION_FLOAT),
    ("heart of stone", "🗿", MOTION_FLOAT),
    ("cold as ice", "🧊", MOTION_FLOAT),
    ("left me alone", "🥀", MOTION_FLOAT),
    ("miss you so", "🥺", MOTION_PULSE),
    
    # Fire & Energy & Hype
    ("on fire", "🔥", MOTION_WIGGLE),
    ("set fire to", "🔥", MOTION_WIGGLE),
    ("catch on fire", "🔥", MOTION_WIGGLE),
    ("burn it down", "🔥", MOTION_WIGGLE),
    ("light it up", "⚡", MOTION_BOUNCE),
    ("light up the", "✨", MOTION_SPARKLE),
    ("blow my mind", "🤯", MOTION_WIGGLE),
    ("rock and roll", "🤘", MOTION_BOUNCE),
    ("rock n roll", "🤘", MOTION_BOUNCE),
    ("bad guy", "😈", MOTION_WIGGLE),
    ("bad girl", "😈", MOTION_WIGGLE),
    ("ride or die", "🏎️", MOTION_BOUNCE),
    ("bullet proof", "🛡️", MOTION_WIGGLE),
    ("here we go", "🚀", MOTION_BOUNCE),
    ("let us go", "🚀", MOTION_BOUNCE),
    ("let's go", "🚀", MOTION_BOUNCE),
    
    # Dreamy, Space & Night
    ("head in the clouds", "☁️", MOTION_FLOAT),
    ("under the stars", "🌌", MOTION_SPARKLE),
    ("in the clouds", "☁️", MOTION_FLOAT),
    ("fly away", "🕊️", MOTION_FLOAT),
    ("middle of the night", "🌙", MOTION_FLOAT),
    ("in the night", "🌙", MOTION_FLOAT),
    ("late night", "🌙", MOTION_FLOAT),
    ("sweet dreams", "🌙", MOTION_FLOAT),
    ("golden hour", "🌅", MOTION_SPARKLE),
    ("lost in the dark", "🕯️", MOTION_FLOAT),
    ("shine bright", "🌟", MOTION_SPARKLE),
    
    # Mind, Communication & Journey
    ("in my head", "🧠", MOTION_FLOAT),
    ("in my mind", "💭", MOTION_FLOAT),
    ("ring my bell", "🔔", MOTION_BOUNCE),
    ("call my phone", "📱", MOTION_BOUNCE),
    ("call me", "📱", MOTION_BOUNCE),
    ("run away", "🏃", MOTION_BOUNCE),
    ("running away", "🏃", MOTION_BOUNCE),
    ("out of time", "⏳", MOTION_FLOAT),
    ("time flies", "⏳", MOTION_FLOAT),
    ("drowning in", "🌊", MOTION_FLOAT),
]

# Layer 2: Semantic Categories & Disambiguated Keywords
SEMANTIC_LEXICON = [
    # 1. Love, Romance & Affection
    ("❤️", MOTION_PULSE, ["love", "loved", "loving", "lover", "darling", "sweetheart", "romance", "romantic", "adore", "beloved"]),
    ("💖", MOTION_PULSE, ["passion", "desire", "crush", "devotion", "fond", "infatuation", "sweetheart"]),
    ("💘", MOTION_PULSE, ["cupid", "arrow", "smitten", "lovesick", "fallen"]),
    ("💋", MOTION_PULSE, ["kiss", "kisses", "kissing", "kissed", "lips", "lipstick", "smooch"]),
    ("🌹", MOTION_FLOAT, ["rose", "roses", "petal", "petals", "bouquet", "flower", "flowers", "bloom"]),
    ("💌", MOTION_PULSE, ["letter", "letters", "postcard", "note", "envelope", "written"]),
    ("💍", MOTION_PULSE, ["marry", "wedding", "proposal", "engaged", "bride", "groom", "vows"]),
    ("🥰", MOTION_PULSE, ["hug", "hugs", "cuddle", "warmth", "gentle", "tender", "cherish"]),
    
    # 2. Heartbreak, Pain & Longing
    ("💔", MOTION_FLOAT, ["heartbreak", "heartbroken", "apart", "broken", "shattered", "scars", "scarred"]),
    ("🥀", MOTION_FLOAT, ["wither", "wilted", "faded", "dying", "decay", "forsaken", "abandoned"]),
    ("😭", MOTION_FLOAT, ["cry", "crying", "cried", "tears", "weep", "weeping", "sob", "sobbing"]),
    ("🥺", MOTION_PULSE, ["please", "begging", "sorry", "forgive", "pardon", "plead"]),
    ("🩹", MOTION_FLOAT, ["heal", "healing", "hurt", "hurting", "pain", "bruise", "bruised", "ache"]),
    ("🖤", MOTION_FLOAT, ["lonely", "alone", "cold", "empty", "emptiness", "sorrow", "regret", "darkness"]),
    
    # 3. Party, Nightlife & Dancing
    ("🪩", MOTION_BOUNCE, ["disco", "party", "club", "groove", "groovin", "mirrors", "boogie"]),
    ("💃", MOTION_BOUNCE, ["dance", "dancing", "danced", "dancer", "dancers", "waltz", "salsa"]),
    ("🎉", MOTION_BOUNCE, ["celebrate", "celebration", "confetti", "cheer", "cheers", "jubilee"]),
    ("🍾", MOTION_BOUNCE, ["champagne", "popping", "bottle", "bottles", "cork", "sparkling"]),
    ("🥂", MOTION_BOUNCE, ["toast", "glasses", "cheers", "clink"]),
    ("🍷", MOTION_BOUNCE, ["wine", "merlot", "cabernet", "sip", "sipping", "glass"]),
    ("🍸", MOTION_BOUNCE, ["cocktail", "martini", "liquor", "vodka", "gin", "drink", "drinking", "drinks", "drunk", "wasted", "tipsy"]),
    ("🍺", MOTION_BOUNCE, ["beer", "brew", "pub", "bar", "ale", "lager"]),
    
    # 4. Music, Performance & Rhythm
    ("🎸", MOTION_BOUNCE, ["guitar", "riff", "chords", "strum", "strumming", "bass", "fender", "gibson", "acoustic", "electric"]),
    ("🎹", MOTION_FLOAT, ["piano", "keys", "keyboard", "melody", "synth", "synthesizer"]),
    ("🥁", MOTION_BOUNCE, ["drum", "drums", "drummer", "snare", "beat", "beats", "percussion", "cymbal", "tempo"]),
    ("🎷", MOTION_BOUNCE, ["sax", "saxophone", "jazz", "brass"]),
    ("🎺", MOTION_BOUNCE, ["trumpet", "horn", "fanfare", "brass"]),
    ("🎻", MOTION_FLOAT, ["violin", "fiddle", "strings", "orchestra", "cello"]),
    ("🎤", MOTION_BOUNCE, ["sing", "singing", "sang", "singer", "vocal", "vocals", "mic", "microphone", "rap", "rapping", "rapper", "verse", "chorus"]),
    ("🎧", MOTION_BOUNCE, ["headphones", "headset", "listen", "listening", "stereo", "audio"]),
    ("🎵", MOTION_PULSE, ["music", "song", "tunes", "tune", "rhythm", "harmony", "musical", "track"]),
    ("🔔", MOTION_BOUNCE, ["bell", "bells", "chime", "chimes", "ring", "ringing", "alarm"]),
    
    # 5. Energy, Fire, Power & Swagger
    ("🔥", MOTION_WIGGLE, ["fire", "flame", "flames", "burn", "burning", "burned", "blaze", "blazing", "hot", "heat", "ignite"]),
    ("⚡", MOTION_BOUNCE, ["lightning", "electric", "electricity", "thunder", "shock", "voltage", "power", "flash", "spark", "energy"]),
    ("👑", MOTION_BOUNCE, ["crown", "king", "queen", "prince", "princess", "royal", "royalty", "reign", "monarch", "throne"]),
    ("💎", MOTION_SPARKLE, ["diamond", "diamonds", "gem", "gems", "jewel", "jewels", "jewelry", "crystal", "precious"]),
    ("💸", MOTION_BOUNCE, ["money", "cash", "dollars", "bills", "rich", "wealth", "wealthy", "fortune", "pay", "paid", "spend"]),
    ("💰", MOTION_BOUNCE, ["gold", "bank", "treasure", "million", "billion", "racks"]),
    ("🏆", MOTION_BOUNCE, ["trophy", "champ", "champion", "winner", "win", "winning", "victory"]),
    ("🕶️", MOTION_BOUNCE, ["shades", "sunglasses", "cool", "swagger", "drip"]),
    ("🚀", MOTION_BOUNCE, ["rocket", "blast", "launch", "soar", "orbit", "liftoff"]),
    ("💣", MOTION_WIGGLE, ["bomb", "dynamite", "explode", "explosion", "boom", "blast"]),
    
    # 6. Night, Space, Mystery & Dreams
    ("🌙", MOTION_FLOAT, ["moon", "luna", "night", "nighttime", "midnight", "crescent", "nocturnal"]),
    ("⭐", MOTION_SPARKLE, ["star", "stars", "starlight", "constellation", "shine", "shining", "glow", "glowing", "wish"]),
    ("🌌", MOTION_SPARKLE, ["galaxy", "milky", "universe", "cosmos", "cosmic", "infinity"]),
    ("🪐", MOTION_FLOAT, ["planet", "planets", "saturn", "jupiter", "mars", "space"]),
    ("☁️", MOTION_FLOAT, ["cloud", "clouds", "cloudy", "haze", "mist", "fog"]),
    ("✨", MOTION_SPARKLE, ["sparkle", "sparkles", "magic", "magical", "glimmer", "shimmer", "radiant", "dazzle"]),
    ("🕯️", MOTION_FLOAT, ["candle", "wick", "wax", "lantern", "flame"]),
    ("💭", MOTION_FLOAT, ["think", "thinking", "thought", "thoughts", "mind", "wonder", "wondering", "dream", "dreaming", "dreams"]),
    
    # 7. Nature, Ocean & Weather
    ("🌊", MOTION_FLOAT, ["wave", "waves", "ocean", "sea", "tide", "tides", "surf", "drown", "deep", "river", "shore", "beach"]),
    ("☀️", MOTION_SPARKLE, ["sun", "sunny", "sunshine", "daylight", "bright", "golden", "summer", "warmth"]),
    ("🌧️", MOTION_FLOAT, ["rain", "raining", "rainy", "storm", "pour", "pouring", "wet", "puddle", "drizzle"]),
    ("⛈️", MOTION_WIGGLE, ["tempest", "hurricane", "tornado", "stormy"]),
    ("❄️", MOTION_FLOAT, ["snow", "snowing", "ice", "icy", "frozen", "freeze", "freezing", "winter", "chilly", "frost"]),
    ("🌸", MOTION_FLOAT, ["blossom", "cherry", "spring", "garden", "meadow", "petals"]),
    ("🍁", MOTION_FLOAT, ["autumn", "fall", "leaf", "leaves", "forest", "trees", "woods"]),
    ("🌈", MOTION_SPARKLE, ["rainbow", "colors", "prism"]),
    
    # 8. Motion, Speed, Travel & Journey
    ("🏎️", MOTION_BOUNCE, ["race", "racing", "speed", "fast", "ferrari", "porsche", "drift", "engine"]),
    ("🚗", MOTION_BOUNCE, ["car", "cars", "drive", "driving", "drove", "ride", "riding", "wheels", "traffic", "road", "street", "highway"]),
    ("🏍️", MOTION_BOUNCE, ["motorcycle", "bike", "harley", "rider", "chopper"]),
    ("✈️", MOTION_FLOAT, ["plane", "airplane", "flight", "flying", "fly", "airport", "runway"]),
    ("🏃", MOTION_BOUNCE, ["run", "running", "ran", "sprint", "chase", "chasing", "flee", "escape"]),
    ("👟", MOTION_BOUNCE, ["shoes", "shoe", "sneakers", "sneaker", "boots", "kicks", "heels", "step", "steps", "walking"]),
    ("💨", MOTION_BOUNCE, ["wind", "breeze", "dash", "rush", "ghost", "gone", "zoom"]),
    
    # 9. Edge, Danger, Dark & Street
    ("💀", MOTION_WIGGLE, ["dead", "death", "die", "dying", "died", "skull", "skeleton", "morbid"]),
    ("⚰️", MOTION_FLOAT, ["coffin", "grave", "cemetery", "tomb", "buried", "funeral"]),
    ("👻", MOTION_FLOAT, ["ghost", "ghosts", "haunt", "haunted", "phantom", "spooky", "spirit"]),
    ("🗡️", MOTION_WIGGLE, ["blade", "knife", "sword", "dagger", "stab", "cut", "wound", "sharp"]),
    ("🔫", MOTION_WIGGLE, ["gun", "guns", "shot", "shoot", "shooting", "bullet", "bullets", "pistol", "trigger", "bang"]),
    ("🐺", MOTION_WIGGLE, ["wolf", "wolves", "howl", "predator", "beast", "fangs"]),
    ("🐍", MOTION_WIGGLE, ["snake", "snakes", "serpent", "poison", "venom", "viper"]),
    ("👁️", MOTION_PULSE, ["eye", "eyes", "stare", "staring", "gaze", "looking", "sight", "watch", "vision"]),
    ("🔒", MOTION_BOUNCE, ["lock", "locked", "cage", "chain", "chains", "trap", "trapped", "safe", "secure"]),
    ("🗝️", MOTION_BOUNCE, ["key", "keys", "unlock", "secret", "clue"]),
    
    # 10. Communication & Objects
    ("📱", MOTION_BOUNCE, ["phone", "cellphone", "mobile", "text", "calling", "screen", "dial"]),
    ("☕", MOTION_FLOAT, ["coffee", "tea", "latte", "espresso", "caffeine", "cup", "mug", "cafe"]),
    ("🚬", MOTION_FLOAT, ["smoke", "smoking", "cigarette", "cigar", "tobacco", "ashes", "inhale", "exhale"]),
    ("💊", MOTION_BOUNCE, ["pill", "pills", "meds", "dose", "drugs", "cure", "remedy"]),
]

# Build reverse keyword mapping
KEYWORD_TO_ENTRY = {}
for emoji, motion, keywords in SEMANTIC_LEXICON:
    for kw in keywords:
        KEYWORD_TO_ENTRY[kw.lower()] = (emoji, motion)

# Disambiguation filters
DISAMBIGUATION_BLACKLIST = {
    "fast": "🍔",
    "play": "🎮",
    "class": "🏀",
    "board": "🛹",
    "rip": "⚰️",
    "blind": "🕶️",
    "bag": "💰",
}

def analyze_lyric_semantics(line_text, focal_word=""):
    """
    Analyzes a lyrical sentence and returns the most evocative, contextually
    appropriate Unicode emoji along with its kinetic motion type.
    
    Returns: (emoji: str, motion_type: int)
    """
    if not line_text or not line_text.strip():
        return None, MOTION_BOUNCE

    clean_text = line_text.lower().strip()
    words = [re.sub(r'[^\w]', '', w).lower() for w in clean_text.split() if w]

    # 1. Multi-word Idiom & Phrase Matching (Highest Priority)
    for idiom, emoji, motion in LYRIC_IDIOMS:
        if idiom in clean_text:
            return emoji, motion

    # 2. Hero Focal Word Semantic Direct Match
    if focal_word:
        clean_focal = re.sub(r'[^\w]', '', focal_word).lower()
        if clean_focal in KEYWORD_TO_ENTRY:
            emoji, motion = KEYWORD_TO_ENTRY[clean_focal]
            return emoji, motion

    # 3. Word-by-Word Scanning with Disambiguation Guardrails
    for w in words:
        if w in KEYWORD_TO_ENTRY:
            emoji, motion = KEYWORD_TO_ENTRY[w]
            if w in DISAMBIGUATION_BLACKLIST and DISAMBIGUATION_BLACKLIST[w] == emoji:
                continue
            return emoji, motion

    # 4. Lyrical Sentiment & Emotion Valence Fallback
    joy_stems = {"good", "great", "better", "alive", "free", "beautiful", "high", "bright", "together", "yeah", "smile", "happy", "sun"}
    sad_stems = {"never", "nowhere", "hard", "dark", "heavy", "trouble", "tired", "fall", "down", "lost", "nobody", "wrong", "sorry"}
    love_stems = {"baby", "babe", "girl", "boy", "forever", "sweet", "touch", "feel", "closer", "stay", "us", "mine"}
    hype_stems = {"stop", "go", "ready", "now", "jump", "bang", "shout", "make", "take", "bring", "push", "move"}

    word_set = set(words)
    if word_set & joy_stems:
        return "✨", MOTION_SPARKLE
    if word_set & sad_stems:
        return "🌧️", MOTION_FLOAT
    if word_set & love_stems:
        return "💖", MOTION_PULSE
    if word_set & hype_stems:
        return "⚡", MOTION_BOUNCE

    # 5. Default Rhythm / Melodic Presence
    return "🎵", MOTION_BOUNCE
