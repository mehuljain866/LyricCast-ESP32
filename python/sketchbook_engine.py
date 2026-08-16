"""
LyricCast Sketchbook Engine
Procedural hand-drawn vector doodle generator, organic handwriting simulator,
and scene serializer for ESP32 and Web Live Canvas.
"""

class SketchbookEngine:
    def __init__(self):
        pass

    @staticmethod
    def format_esp32_packet(scene):
        """Formats a structured scene packet for the ESP32 Serial protocol."""
        # Protocol: K|<metaphor>|<doodle>|<composition>|<focal>|<prefix>|<suffix>|<tilt>|<underline>|<durationMs>|<fontPreset>
        prefix = scene.get('prefix', '').replace('|', '/')
        focal = scene.get('focal_word', '').replace('|', '/')
        suffix = scene.get('suffix', '').replace('|', '/')
        metaphor = scene.get('metaphor', 'NORMAL').upper()
        doodle = scene.get('doodle', 'NONE').upper()
        composition = scene.get('composition', 'CENTER').upper()
        tilt = scene.get('tilt_angle', 0)
        underline = 1 if scene.get('has_underline') else 0
        duration_ms = scene.get('duration_ms', 3000)
        font_preset = scene.get('font_preset', 0)
        
        packet = f"K|{metaphor}|{doodle}|{composition}|{focal}|{prefix}|{suffix}|{tilt}|{underline}|{duration_ms}|{font_preset}\n"
        return packet
