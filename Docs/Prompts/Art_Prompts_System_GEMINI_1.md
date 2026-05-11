# Art_Prompts_System_NEW.md

## 1. Project Vision
This project is a **2.5D side-scrolling platformer** set in a **mature, stylized 3D fantasy world**. The environment and assets are designed as **functional in-game elements** rather than cinematic illustrations. 

The visual direction is inspired by the atmosphere of **Heroes of Might and Magic II**, interpreted as **high-fidelity hand-painted 3D surfaces**, not pixel art and not cartoon animation.

The project strictly avoids:
*   Cute cartoon proportions
*   Rubbery forms
*   Children’s animation aesthetics

**Compositional References:**
*   **Ori and the Blind Forest:** For ultra-wide layered composition and atmospheric scale.
*   **Inside:** For functional level staging and readable playable space.

---

## 2. Core Style Rules
*   **Mature Fantasy Atmosphere:** Elegant, majestic, and handcrafted.
*   **Direct Gameplay/Asset Focus:** Prioritize level readability and technical asset clarity.
*   **High-Fidelity Surfaces:** Matte hand-painted 3D materials (not plastic or glossy).
*   **Strict Exclusions:** No cartoon, no cute, no chibi, no settlement props in nature.

---


## INSTRUCTIONS (How to Build)

### How to assemble a Wide Shot Prompt:
1.  **Start with:** [Module A: Master Environment Prompt] (Заполните переменные {BIOME}, {MOOD} и т.д.).
2.  **Add:** [Layout 1: Wide Shot].
3.  **End with:** [Negative 1: Universal] + [Negative 2: Natural Biome Exclusions].

### How to assemble a Creature Turnaround Prompt:
1.  **Start with:** [Module B: Master Creature Concept Prompt] (Укажите {CREATURE} и {BIOME_SPECIFICS}).
2.  **Add:** [Layout 2: Creature Turnaround].
3.  **End with:** [Negative 1: Universal] + [Negative 3: Orthographic Exclusions].

---



## 3. Global Variables
Используйте эти переменные для настройки контекста промпта:

*   `{PROJECT_STYLE}` = mature fantasy, HoMM II inspired, hand-painted 3D assets, elegant and atmospheric
*   `{BIOME}` = forest / swamp / mountains / snowlands / burning forest / village / city
*   `{BIOME_SPECIFICS}` = (например: layered stone, jagged granite plates, mossy mountain rock)
*   `{BIOME_EXCLUSIONS}` = (например: no houses, no roofs, no villages, no fences, no signs)
*   `{MOOD}` = calm / majestic / mysterious / slightly cold / dangerous
*   `{TIME_OF_DAY}` = morning / noon / dusk / night
*   `{SEASON}` = spring / summer / autumn / winter
*   `{CREATURE}` = (например: Giant Stone Crab)
*   `{CREATURE_DESCRIPTION}` = (например: Giant Crab made of stones and dry wood, with a shell and claws formed from large, irregularly shaped stones)

---

## 4. Master Modules (The "Constructor")

### Module A: Master Environment Prompt (Base for Levels)
> Direct gameplay screencap of a 2.5D side-scrolling platformer level. Create a stylized 3D fantasy environment designed as a functional in-game level mockup with an orthographic-style side-view perspective. The visual direction is mature fantasy, inspired by the atmospheric hand-painted 3D surfaces of Heroes of Might and Magic II, but rendered as high-fidelity 3D assets. Strictly avoid all cartoon aesthetics, cute proportions, and soft animation-style forms. Use Ori and the Blind Forest and Inside as references for level depth and functional staging.

### Module B: Master Creature Concept Prompt (Base for Assets)
> Orthographic character turnaround sheet for an original {CREATURE}. {CREATURE_DESCRIPTION}. This asset is designed for a mature fantasy world inspired by the slightly flattened painterly rendering and rich color harmony of Heroes of Might and Magic II. The creature must look like a stylized hand-painted 3D model with clean silhouettes and believable fantasy anatomy. Strictly avoid photorealism and glossy plastic textures.


---

## 5. Layout Modules

### Layout 1: Wide Shot (Full Level View)
> Create an ultra-wide panoramic establishing composition of a 2.5D side-scrolling fantasy level set in {BIOME}. Show foreground, midground, and background together in one cohesive scene. Use an extreme horizontal framing with a long-distance camera placement. The camera must be almost level with the horizon (10-degree downward tilt). The playable path must be clearly readable as a long, unobstructed horizontal midground plane through the center of the image. Foreground elements must be pushed to the extreme screen edges.

### Layout 2: Creature Turnaround (The Orthographic Sheet)
> Show the {CREATURE} as a structured reference sheet on a solid neutral grey background with no environment or scenery. The image must contain four full-body views of the same {CREATURE}, perfectly aligned horizontally, demonstrating a unified design:
> 1. **Strict Front View:** (90-degree angle, symmetrical).
> 2. **Strict Right Side View:** (90-degree profile).
> 3. **Strict Back View:** (90-degree angle, from behind).
> 4. **Strict Top-Down View:** (90-degree angle, from above).
> All views must be pure orthographic projections with no perspective distortion. High-detail 8K resolution, sharp focus.

---

## 6. Negative Prompts

### Negative 1: Universal (Always Use)
> (photorealism:1.4), (realism:1.4), (3d render:1.2), (background:1.5), (scenery:1.5), (environment:1.5), (concept art sketch:1.2), (watercolor:1.2), oil painting, cartoon, animated film still, cute style, chibi, exaggerated facial expressions, thick outlines, rubbery shapes, childish proportions, pixel art, low poly, anime, sci-fi, futuristic objects, modern objects, UI, watermark, logo, random text, messy composition, clutter, harsh directional shadows, cinematic lighting, lens flare, extreme depth of field, blurry image, overexposed highlights, neon colors, plastic look, broken anatomy, extra limbs, duplicated limbs, inconsistent perspective, cropped subject, noisy background, AI artifacts.

### Negative 2: Natural Biome Exclusions
> houses, roofs, buildings, villages, towns, cities, fences, windows, doors, chimneys, lamps, signs, roads, settlement props, urban elements

### Negative 3: Orthographic/Asset Exclusions (For Creature Sheets)
> (background:1.5), (scenery:1.5), (environment:1.5), mountains, sky, grass, (perspective:1.3), (3/4 view:1.3)

---