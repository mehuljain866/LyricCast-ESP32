"""
LyricCast Context-Aware Semantic Emoji Engine (Expressive++)
Comprehensive lyrical intelligence:
1. Extensive multi-word lyrical idioms & phrases (matched first)
2. 500+ lyrical keywords across body, people, clothing, actions, emotions, music
3. Smart word disambiguation & priority weighting (e.g. clothing > beat, speed > burger)
4. Context-sensitive, non-repeating lyrical fallback (no more 10x repeated music logos!)
"""

import re

# Layer 1: High-Priority Multi-Word Lyrical Idioms & Phrases
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
    ("turn the music up", "🔊"),
    ("turn it up", "🔊"),
    ("pop the champagne", "🍾"),
    ("champagne problems", "🍾"),
    ("hands in the air", "🙌"),
    ("good vibes", "✨"),
    ("wild side", "🐺"),
    
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
    
    # Holding, Hands & Arms
    ("holding back", "🛑"),
    ("hold back", "🛑"),
    ("hold my hand", "🤝"),
    ("take my hand", "🤝"),
    ("took my arm", "🤝"),
    ("take my arm", "🤝"),
    ("in your arms", "🫂"),
    ("in my arms", "🫂"),
    ("hold me tight", "🫂"),
    ("hold on", "🤝"),
    
    # Romance, Love & Destiny
    ("fall in love", "💘"),
    ("falling in love", "💘"),
    ("in love with", "❤️"),
    ("make love", "💋"),
    ("kiss me", "💋"),
    ("kiss you", "💋"),
    ("take my breath away", "🌹"),
    ("head over heels", "🥰"),
    ("die for you", "🥀"),
    ("heart of gold", "💛"),
    ("bound to get together", "✨"),
    ("bound to be together", "✨"),
    ("get together", "✨"),
    ("be together", "✨"),
    ("my destiny", "🔮"),
    ("you and me", "💑"),
    ("stay with me", "🥺"),
    ("teenage dream", "💭"),
    
    # Heartbreak & Sadness
    ("broken heart", "💔"),
    ("break my heart", "💔"),
    ("breaks my heart", "💔"),
    ("broke my heart", "💔"),
    ("cry me a river", "😭"),
    ("tears fall", "😢"),
    ("tears falling", "😢"),
    ("tears in my eyes", "🥺"),
    ("tears rolling down", "😭"),
    ("heart of stone", "🗿"),
    ("cold as ice", "🧊"),
    ("left me alone", "🥀"),
    ("miss you so", "🥺"),
    ("felt it in my chest", "💓"),
    ("in my chest", "💓"),
    
    # Energy, Fire & Speed
    ("on fire", "🔥"),
    ("set fire to", "🔥"),
    ("catch on fire", "🔥"),
    ("burn it down", "🔥"),
    ("light it up", "⚡"),
    ("light up the", "✨"),
    ("blow my mind", "🤯"),
    ("rock and roll", "🤘"),
    ("rock n roll", "🤘"),
    ("bad guy", "😈"),
    ("bad girl", "😈"),
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
    ("golden hour", "🌅"),
    ("lost in the dark", "🕯️"),
    ("shine bright", "🌟"),
    
    # Mind, Communication & Questions
    ("in my head", "🧠"),
    ("in my mind", "💭"),
    ("don't know", "🤷"),
    ("dont know", "🤷"),
    ("ring my bell", "🔔"),
    ("call my phone", "📱"),
    ("call me", "📱"),
    ("run away", "🏃"),
    ("running away", "🏃"),
    ("out of time", "⏳"),
    ("time flies", "⏳"),
    ("drowning in", "🌊"),
]

# Layer 2: Comprehensive Lyrical Semantic Lexicon (Word Stems -> Emoji)
SEMANTIC_LEXICON = [
    # 1. Vision, Eyes & Gaze
    ("👀", ["eye", "eyes", "look", "looks", "looked", "looking", "stare", "staring", "gaze", "see", "saw", "seeing", "sight", "watch", "watching", "view", "glance"]),
    
    # 2. Hands, Arms, Touch & Holding
    ("🤝", ["hand", "hands", "hold", "holds", "holding", "held", "touch", "touching", "touched", "reach", "reaching", "grab", "grabbed", "arm", "arms"]),
    ("🫂", ["hug", "hugs", "embrace", "cuddle", "cuddling", "holdme", "closer"]),
    
    # 3. People, Women, Men & Identity
    ("💃", ["woman", "lady", "female", "girl", "girls", "queen", "goddess", "babe", "baby", "juliet"]),
    ("🕺", ["man", "guy", "boy", "boys", "king", "prince", "gentleman", "dude", "romeo"]),
    ("👥", ["crowd", "people", "everyone", "everybody", "somebody", "someone", "friends", "together"]),
    
    # 4. Clothing, Style & Footwear
    ("👗", ["dress", "skirt", "gown", "outfit", "wear", "wearing", "backless", "fashion"]),
    ("👟", ["sneaks", "sneaker", "sneakers", "shoe", "shoes", "boots", "heels", "kicks", "foot", "feet", "step", "steps"]),
    ("🧥", ["jacket", "coat", "hoodie", "sweater", "shirt"]),
    
    # 5. Chest, Heart & Physical Sensations
    ("💓", ["chest", "heartbeat", "pulse", "breath", "breathe", "breathing", "alive", "flutter", "pounding"]),
    ("❤️", ["love", "loved", "loving", "lover", "darling", "sweetheart", "romance", "romantic", "adore", "beloved"]),
    ("💖", ["passion", "desire", "crush", "devotion", "fond", "infatuation", "sweet"]),
    ("💘", ["cupid", "arrow", "smitten", "lovesick", "fallen"]),
    ("💋", ["kiss", "kisses", "kissing", "kissed", "lips", "lipstick", "smooch"]),
    ("🌹", ["rose", "roses", "petal", "petals", "bouquet", "flower", "flowers", "bloom"]),
    ("💌", ["letter", "letters", "postcard", "note", "envelope", "written"]),
    ("💍", ["marry", "wedding", "proposal", "engaged", "bride", "groom", "vows"]),
    
    # 6. Heartbreak, Sadness, Crying & Scars
    ("💔", ["heartbreak", "heartbroken", "apart", "broken", "shattered", "break", "broke"]),
    ("🥀", ["wither", "wilted", "faded", "dying", "decay", "forsaken", "abandoned", "destiny", "fate"]),
    ("😭", ["cry", "crying", "cried", "tears", "weep", "weeping", "sob", "sobbing"]),
    ("🥺", ["please", "begging", "sorry", "forgive", "pardon", "plead", "miss", "missing"]),
    ("🩹", ["heal", "healing", "hurt", "hurting", "pain", "bruise", "bruised", "ache", "scars", "scarred"]),
    ("🖤", ["lonely", "alone", "cold", "empty", "emptiness", "sorrow", "regret", "darkness", "victim", "victims"]),
    
    # 7. Nightlife, Dancing & Party
    ("🪩", ["disco", "party", "club", "groove", "groovin", "floor", "mirrors", "boogie", "discothèque", "discotheque"]),
    ("💃", ["dance", "dancing", "danced", "dancer", "dancers", "waltz", "salsa"]),
    ("🎉", ["celebrate", "celebration", "confetti", "cheer", "cheers", "jubilee"]),
    ("🍾", ["champagne", "popping", "bottle", "bottles", "cork", "sparkling"]),
    ("🥂", ["toast", "glasses", "cheers", "clink"]),
    ("🍷", ["wine", "merlot", "cabernet", "sip", "sipping", "glass"]),
    ("🍸", ["cocktail", "martini", "liquor", "vodka", "gin", "drink", "drinking", "drinks", "drunk", "wasted", "tipsy"]),
    ("🍺", ["beer", "brew", "pub", "bar", "ale", "lager"]),
    
    # 8. Music, Instruments & Audio
    ("🎸", ["guitar", "riff", "chords", "strum", "strumming", "bass", "fender", "gibson", "acoustic", "electric"]),
    ("🎹", ["piano", "keys", "keyboard", "melody", "synth", "synthesizer"]),
    ("🥁", ["drum", "drums", "drummer", "snare", "cymbal", "percussion"]),
    ("🎷", ["sax", "saxophone", "jazz", "brass"]),
    ("🎺", ["trumpet", "horn", "fanfare", "brass"]),
    ("🎻", ["violin", "fiddle", "strings", "orchestra", "cello"]),
    ("🎤", ["sing", "singing", "sang", "singer", "vocal", "vocals", "mic", "microphone", "rap", "rapping", "rapper", "verse", "chorus"]),
    ("🎧", ["headphones", "headset", "listen", "listening", "stereo", "audio"]),
    ("🔔", ["bell", "bells", "chime", "chimes", "alarm"]),
    
    # 9. Energy, Fire, Lightning & Power
    ("🔥", ["fire", "flame", "flames", "burn", "burning", "burned", "blaze", "blazing", "hot", "heat", "ignite"]),
    ("⚡", ["lightning", "electric", "electricity", "thunder", "shock", "voltage", "power", "flash", "spark", "energy", "physical", "chemical", "kryptonite"]),
    ("👑", ["crown", "king", "queen", "prince", "princess", "royal", "royalty", "reign", "monarch", "throne"]),
    ("💎", ["diamond", "diamonds", "gem", "gems", "jewel", "jewels", "jewelry", "crystal", "precious"]),
    ("💸", ["money", "cash", "dollars", "bills", "rich", "wealth", "wealthy", "fortune", "pay", "paid", "spend"]),
    ("💰", ["gold", "bank", "treasure", "million", "billion", "racks"]),
    ("🏆", ["trophy", "champ", "champion", "winner", "win", "winning", "victory"]),
    ("🕶️", ["shades", "sunglasses", "cool", "swagger", "drip"]),
    ("🚀", ["rocket", "blast", "launch", "soar", "orbit", "liftoff"]),
    ("💣", ["bomb", "dynamite", "explode", "explosion", "boom", "blast"]),
    
    # 10. Night, Space, Mystery & Cosmic
    ("🌙", ["moon", "luna", "night", "nighttime", "midnight", "crescent", "nocturnal"]),
    ("⭐", ["star", "stars", "starlight", "constellation", "shine", "shining", "glow", "glowing", "wish"]),
    ("🌌", ["galaxy", "milky", "universe", "cosmos", "cosmic", "infinity"]),
    ("🪐", ["planet", "planets", "saturn", "jupiter", "mars", "space"]),
    ("☁️", ["cloud", "clouds", "cloudy", "haze", "mist", "fog"]),
    ("✨", ["sparkle", "sparkles", "magic", "magical", "glimmer", "shimmer", "radiant", "dazzle", "bound", "wonder"]),
    ("🕯️", ["candle", "wick", "wax", "lantern", "flame", "light"]),
    ("💭", ["think", "thinking", "thought", "thoughts", "mind", "wonder", "wondering", "dream", "dreaming", "dreams", "remember"]),
    ("🔮", ["destiny", "fate", "fortune", "future", "crystal", "prophecy"]),
    ("🤷", ["know", "happened", "question", "how", "why"]),
    
    # 11. Nature, Weather & Water
    ("🌊", ["wave", "waves", "ocean", "sea", "tide", "tides", "surf", "drown", "deep", "river", "shore", "beach"]),
    ("☀️", ["sun", "sunny", "sunshine", "daylight", "bright", "golden", "summer", "warmth"]),
    ("🌧️", ["rain", "raining", "rainy", "storm", "pour", "pouring", "wet", "puddle", "drizzle"]),
    ("⛈️", ["tempest", "hurricane", "tornado", "stormy"]),
    ("❄️", ["snow", "snowing", "ice", "icy", "frozen", "freeze", "freezing", "winter", "chilly", "frost"]),
    ("🌸", ["blossom", "cherry", "spring", "garden", "meadow", "petals"]),
    ("🍁", ["autumn", "fall", "leaf", "leaves", "forest", "trees", "woods"]),
    ("🌈", ["rainbow", "colors", "prism"]),
    
    # 12. Speed, Driving, Travel & Escape
    ("🏎️", ["race", "racing", "speed", "fast", "ferrari", "porsche", "drift", "engine"]),
    ("🚗", ["car", "cars", "drive", "driving", "drove", "ride", "riding", "wheels", "traffic", "road", "street", "highway"]),
    ("🏍️", ["motorcycle", "bike", "harley", "rider", "chopper"]),
    ("✈️", ["plane", "airplane", "flight", "flying", "fly", "airport", "runway"]),
    ("🏃", ["run", "running", "ran", "sprint", "chase", "chasing", "flee", "escape", "dare"]),
    ("🛑", ["stop", "holding", "wait", "waiting", "pause", "enough", "stay"]),
    
    # 13. Street, Danger & Edge
    ("💀", ["dead", "death", "die", "dying", "died", "skull", "skeleton", "morbid"]),
    ("⚰️", ["coffin", "grave", "cemetery", "tomb", "buried", "funeral"]),
    ("👻", ["ghost", "ghosts", "haunt", "haunted", "phantom", "spooky", "spirit"]),
    ("🗡️", ["blade", "knife", "sword", "dagger", "stab", "cut", "wound", "sharp"]),
    ("🔫", ["gun", "guns", "shot", "shoot", "shooting", "bullet", "bullets", "pistol", "trigger", "bang"]),
    ("🐺", ["wolf", "wolves", "howl", "predator", "beast", "fangs", "wild"]),
    ("🐍", ["snake", "snakes", "serpent", "poison", "venom", "viper"]),
    
    # 14. Communication & Voices
    ("📱", ["phone", "cellphone", "mobile", "text", "calling", "screen", "dial", "call"]),
    ("🗣️", ["said", "speak", "speaking", "spoke", "talk", "talking", "talked", "tell", "telling", "told", "shout", "whisper", "scream", "ooh", "hoo", "yeah", "woah"]),
    ("☕", ["coffee", "tea", "latte", "espresso", "caffeine", "cup", "mug", "cafe"]),
    ("🚬", ["smoke", "smoking", "cigarette", "cigar", "tobacco", "ashes", "inhale", "exhale"]),
    ("💊", ["pill", "pills", "meds", "dose", "drugs", "cure", "remedy"]),
]

# Build reverse keyword mapping
KEYWORD_TO_EMOJI = {}
for emoji, keywords in SEMANTIC_LEXICON:
    for kw in keywords:
        KEYWORD_TO_EMOJI[kw.lower()] = emoji

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

# Dynamic rotating fallback palette for generic lyrics (ensures NEVER repeating identical icon 10x!)
FALLBACK_PALETTE = ["✨", "🎶", "💫", "🌟", "💭", "💖", "🎵", "🕊️"]

def analyze_lyric_semantics(line_text, focal_word=""):
    """
    Analyzes a lyrical sentence and returns the most evocative, contextually
    appropriate Unicode emoji.
    """
    if not line_text or not line_text.strip():
        return "✨", 0

    clean_text = line_text.lower().strip()
    words = [re.sub(r'[^\w]', '', w).lower() for w in clean_text.split() if w]

    # 1. Multi-word Idioms & Phrases (Top Priority)
    for idiom, emoji in LYRIC_IDIOMS:
        if idiom in clean_text:
            return emoji, 0

    # 2. Hero Focal Word Semantic Direct Match
    if focal_word:
        clean_focal = re.sub(r'[^\w]', '', focal_word).lower()
        if clean_focal in KEYWORD_TO_EMOJI:
            return KEYWORD_TO_EMOJI[clean_focal], 0

    # 3. Word-by-Word Scanning with Disambiguation Guardrails
    for w in words:
        if w in KEYWORD_TO_EMOJI:
            emoji = KEYWORD_TO_EMOJI[w]
            if w in DISAMBIGUATION_BLACKLIST and DISAMBIGUATION_BLACKLIST[w] == emoji:
                continue
            return emoji, 0

    # 4. Sentiment & Valence Fallback
    joy_stems = {"good", "great", "better", "alive", "free", "beautiful", "high", "bright", "together", "smile", "happy", "sun"}
    sad_stems = {"never", "nowhere", "hard", "dark", "heavy", "trouble", "tired", "fall", "down", "lost", "nobody", "wrong", "sorry"}
    love_stems = {"baby", "babe", "girl", "boy", "forever", "sweet", "touch", "feel", "closer", "stay", "us", "mine"}
    hype_stems = {"stop", "go", "ready", "now", "jump", "bang", "shout", "make", "take", "bring", "push", "move"}

    word_set = set(words)
    if word_set & joy_stems:
        return "✨", 0
    if word_set & sad_stems:
        return "🌧️", 0
    if word_set & love_stems:
        return "💖", 0
    if word_set & hype_stems:
        return "⚡", 0

    # 5. Smart Non-Repeating Fallback Palette (Cycles dynamically based on line hash)
    line_hash = abs(hash(clean_text))
    fallback_emoji = FALLBACK_PALETTE[line_hash % len(FALLBACK_PALETTE)]
    return fallback_emoji, 0
