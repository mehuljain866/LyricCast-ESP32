"""
LyricCast Sketchbook Engine
Procedural hand-drawn vector doodle generator, organic handwriting simulator,
and scene serializer for ESP32 and Web Live Canvas.
"""

import math
import random

class SketchbookEngine:
    def __init__(self):
        pass

    @staticmethod
    def generate_heart_points(cx, cy, radius=8, wobble=True):
        """Generates parametric heart points with organic hand-drawn wobble."""
        points = []
        steps = 16
        for i in range(steps):
            t = (i / steps) * 2 * math.pi
            # Parametric heart equation
            x = 16 * (math.sin(t) ** 3)
            y = -(13 * math.cos(t) - 5 * math.cos(2*t) - 2 * math.cos(3*t) - math.cos(4*t))
            
            # Normalize to radius
            px = cx + (x / 16.0) * radius
            py = cy + (y / 16.0) * radius
            
            if wobble:
                px += random.uniform(-0.6, 0.6)
                py += random.uniform(-0.6, 0.6)
                
            points.append((round(px, 1), round(py, 1)))
        return points

    @staticmethod
    def generate_star_points(cx, cy, outer_r=7, inner_r=3, spikes=5):
        """Generates starry sparkle vector points."""
        points = []
        rot = random.uniform(0, math.pi / 4)
        for i in range(spikes * 2):
            r = outer_r if (i % 2 == 0) else inner_r
            angle = (i * math.pi / spikes) + rot
            x = cx + math.cos(angle) * r + random.uniform(-0.4, 0.4)
            y = cy + math.sin(angle) * r + random.uniform(-0.4, 0.4)
            points.append((round(x, 1), round(y, 1)))
        return points

    @staticmethod
    def generate_arrow_points(x1, y1, x2, y2, curve=4):
        """Generates a curved hand-drawn arrow pointing to a word."""
        mid_x = (x1 + x2) / 2 + random.uniform(-curve, curve)
        mid_y = (y1 + y2) / 2 + random.uniform(-curve, curve)
        
        # Arrowhead angle
        angle = math.atan2(y2 - mid_y, x2 - mid_x)
        head_len = 5
        
        left_wing = (x2 - head_len * math.cos(angle - 0.5), y2 - head_len * math.sin(angle - 0.5))
        right_wing = (x2 - head_len * math.cos(angle + 0.5), y2 - head_len * math.sin(angle + 0.5))
        
        return {
            "shaft": [(x1, y1), (round(mid_x, 1), round(mid_y, 1)), (x2, y2)],
            "head": [(round(left_wing[0], 1), round(left_wing[1], 1)), (x2, y2), (round(right_wing[0], 1), round(right_wing[1], 1))]
        }

    @staticmethod
    def generate_underline_points(x1, x2, y, roughness=1.5):
        """Generates an organic, imperfect handwritten underline."""
        points = []
        step = max(4, (x2 - x1) // 5)
        for x in range(int(x1), int(x2) + 1, int(step)):
            jitter = random.uniform(-roughness, roughness)
            points.append((x, round(y + jitter, 1)))
        if points[-1][0] < x2:
            points.append((x2, round(y + random.uniform(-0.5, 0.5), 1)))
        return points

    @staticmethod
    def format_esp32_packet(scene):
        """Formats a structured scene packet for the ESP32 Serial protocol."""
        # Protocol: K|<type>|<metaphor>|<doodle>|<focal>|<prefix>|<suffix>|<tilt>|<underline>
        prefix = scene.get('prefix', '').replace('|', '/')
        focal = scene.get('focal_word', '').replace('|', '/')
        suffix = scene.get('suffix', '').replace('|', '/')
        metaphor = scene.get('metaphor', 'normal').upper()
        doodle = scene.get('doodle', 'none').upper()
        tilt = scene.get('tilt_angle', 0)
        underline = 1 if scene.get('has_underline') else 0
        
        packet = f"K|{metaphor}|{doodle}|{focal}|{prefix}|{suffix}|{tilt}|{underline}\n"
        return packet
