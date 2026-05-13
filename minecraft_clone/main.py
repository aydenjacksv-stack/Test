from ursina import *
from ursina.prefabs.first_person_controller import FirstPersonController
import math
import random

try:
    import noise
    HAS_NOISE = True
except ImportError:
    HAS_NOISE = False

app = Ursina(
    title='Minecraft Clone',
    borderless=False,
    fullscreen=False,
    size=(1280, 720),
)

# ── Couleurs des blocs ──────────────────────────────────────────────────────
BLOCK_COLORS = {
    1: color.rgb(86, 125, 70),
    2: color.rgb(134, 96, 67),
    3: color.rgb(128, 128, 128),
    4: color.rgb(101, 67, 33),
    5: color.rgb(34, 139, 34),
    6: color.rgb(240, 230, 140),
    7: color.rgb(200, 230, 255),
    8: color.rgb(180, 80, 60),
}

BLOCK_NAMES = {
    1: 'Herbe',
    2: 'Terre',
    3: 'Pierre',
    4: 'Bois',
    5: 'Feuilles',
    6: 'Sable',
    7: 'Verre',
    8: 'Brique',
}

placed_blocks = {}


class Block(Button):
    def __init__(self, position=(0, 0, 0), block_type=1):
        super().__init__(
            parent=scene,
            position=position,
            model='cube',
            origin_y=0.5,
            texture='white_cube',
            color=BLOCK_COLORS.get(block_type, color.white),
            highlight_color=color.rgba(255, 255, 255, 120),
        )
        self.block_type = block_type
        placed_blocks[tuple(int(v) for v in position)] = self

    def input(self, key):
        if self.hovered:
            if key == 'left mouse down':
                pos = tuple(int(v) for v in self.position)
                placed_blocks.pop(pos, None)
                destroy(self)

            if key == 'right mouse down':
                new_pos = self.position + mouse.normal
                key_pos = tuple(int(v) for v in new_pos)
                if key_pos not in placed_blocks:
                    Block(position=new_pos, block_type=game.selected_block)


# ── Génération du terrain ───────────────────────────────────────────────────
def generate_terrain(size=20):
    for z in range(-size, size):
        for x in range(-size, size):
            if HAS_NOISE:
                h = int(noise.pnoise2(x * 0.08, z * 0.08, octaves=4, persistence=0.5) * 6) + 3
                biome = noise.pnoise2(x * 0.02 + 100, z * 0.02 + 100)
            else:
                h = int(math.sin(x * 0.3) * 2 + math.cos(z * 0.3) * 2) + 3
                biome = 0

            surface = 6 if biome > 0.3 else 1

            Block(position=(x, h, z), block_type=surface)
            for dy in range(1, max(h + 6, 4)):
                btype = 2 if dy <= 2 else 3
                Block(position=(x, h - dy, z), block_type=btype)

            if surface == 1 and HAS_NOISE:
                tree_noise = noise.pnoise2(x * 0.5 + 50, z * 0.5 + 50)
                if tree_noise > 0.35 and random.random() > 0.85:
                    _make_tree(x, h + 1, z)


def _make_tree(x, y, z):
    for i in range(4):
        Block(position=(x, y + i, z), block_type=4)
    for dx in range(-2, 3):
        for dz in range(-2, 3):
            for dy in range(3, 6):
                if abs(dx) + abs(dz) + abs(dy - 4) < 4:
                    key = (x + dx, y + dy, z + dz)
                    if key not in placed_blocks:
                        Block(position=key, block_type=5)


# ── HUD & logique de jeu ────────────────────────────────────────────────────
class Game(Entity):
    def __init__(self):
        super().__init__()
        self.selected_block = 1
        self.hotbar_blocks = list(range(1, 9))
        self.hotbar_index = 0
        self._build_hud()

    def _build_hud(self):
        Sky(color=color.rgb(135, 206, 235))

        # Réticule
        Entity(parent=camera.ui, model='quad', color=color.white,
               scale=(0.002, 0.018), position=(0, 0, -1))
        Entity(parent=camera.ui, model='quad', color=color.white,
               scale=(0.018, 0.002), position=(0, 0, -1))

        # Fond de la barre de raccourcis
        Entity(parent=camera.ui, model='quad',
               color=color.rgba(0, 0, 0, 140),
               scale=(0.58, 0.065), position=(0, -0.45))

        # Cases colorées
        self.slots = []
        for i, bt in enumerate(self.hotbar_blocks):
            x = (i - 3.5) * 0.067
            slot = Entity(parent=camera.ui, model='quad',
                          color=BLOCK_COLORS.get(bt, color.white),
                          scale=0.052, position=(x, -0.45))
            self.slots.append(slot)
            Text(str(i + 1), parent=camera.ui,
                 position=(x - 0.016, -0.422), scale=0.55, color=color.white)

        # Indicateur de sélection
        self.selector = Entity(parent=camera.ui, model='quad',
                               color=color.rgba(255, 255, 255, 60),
                               scale=0.058,
                               position=(-3.5 * 0.067, -0.45))

        # Nom du bloc actif
        self.name_text = Text(BLOCK_NAMES[1], parent=camera.ui,
                              position=(0, -0.385), scale=1.1,
                              color=color.white, origin=(0, 0))

        # Instructions
        Text('ZQSD/WASD: Déplacer | Souris: Regarder | Clic G: Casser | '
             'Clic D: Placer | 1-8 / Molette: Bloc | ESC: Quitter',
             parent=camera.ui, position=(0, 0.46),
             scale=0.55, color=color.white, origin=(0, 0))

    def update(self):
        for i in range(8):
            if held_keys[str(i + 1)]:
                self._select(i)
        if mouse.wheel:
            self._select((self.hotbar_index - int(mouse.wheel)) % 8)

    def _select(self, index):
        self.hotbar_index = index % 8
        self.selected_block = self.hotbar_blocks[self.hotbar_index]
        self.selector.x = (self.hotbar_index - 3.5) * 0.067
        self.name_text.text = BLOCK_NAMES.get(self.selected_block, '?')


# ── Lancement ───────────────────────────────────────────────────────────────
game = Game()

player = FirstPersonController(position=(0, 12, 0), jump_height=1.5, speed=5)
player.gravity = 1

print('Generation du monde...')
generate_terrain()
print('Monde pret !')

app.run()
