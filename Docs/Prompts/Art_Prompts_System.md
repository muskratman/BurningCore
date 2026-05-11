# Image Prompt System for a 2.5D Fantasy Platformer


## Рекомендации по структуре запроса
При составлении окончательного запроса придерживайтесь следующего порядка:

```text
[master_prompt]
+ [biome module]
+ [layer or subject module]
+ [negative_prompt]
```

Пример:

```text
master_prompt
+ foreground_prompt with {BIOME}=mountains
+ negative_prompt
```

или

```text
master_prompt
+ creature_concept_prompt with {BIOME}=forest
+ negative_prompt
```

---

## Глобальные переменные
Используйте эти переменные в запросах, чтобы обеспечить модульность системы.

```text
{PROJECT_STYLE} = bright fairytale fantasy, storybook illustration, HoMM II inspired, hand-painted 3D, not pixel art
{BIOME} = forest / swamp / mountains / snowlands / burning forest / village / city
{LAYER} = foreground / midground / background / wide shot
{SUBJECT} = main object or character
{MOOD} = calm / magical / mysterious / cozy / heroic / dangerous
{TIME_OF_DAY} = morning / noon / dusk / night
{SEASON} = spring / summer / autumn / winter
{MATERIALS} = stone, wood, moss, bark, cloth, metal, etc.
{SCALE} = tiny / human-sized / giant / monumental
{FUNCTION} = decorative / interactive / traversal / landmark
```

---

## Master Prompt

```text
Create a stylized 3D fairytale fantasy environment or asset for a 2.5D side-scrolling platformer. The visual direction is inspired by Heroes of Might and Magic II: The Succession Wars, interpreted as bright storybook fantasy with a strong book-illustration feel, rich color harmony, and hand-painted 3D surfaces, not pixel art.

Use Ori and the Blind Forest as a reference for camera depth, layered composition, and atmospheric staging. Use Inside as a reference for level depth, readable scene structure, and functional background design.

The world should feel magical, elegant, and cohesive, with slightly flattened painterly rendering on 3D forms. Keep the composition readable from a side-view gameplay camera, with clear separation between foreground, midground, and background. Use {BIOME} as the main setting, with {MOOD}, {TIME_OF_DAY}, and {SEASON} defining the atmosphere.

Focus on clean silhouettes, believable fantasy scale, stylized proportions, natural material variation, soft atmospheric perspective, and production-ready concept art quality. The result should look like a unified game art asset from the same world.

No pixel art, no photorealism, no modern objects, no sci-fi elements, no cluttered composition.
```

---

## Negative Prompt

```text
Negative prompt:
photorealism, pixel art, low poly, anime, sci-fi, futuristic objects, modern objects, UI, watermark, logo, random text, messy composition, clutter, harsh directional shadows, cinematic lighting, lens flare, extreme depth of field, blurry image, overexposed highlights, neon colors, plastic look, broken anatomy, extra limbs, duplicated limbs, inconsistent perspective, cropped subject, noisy background, gore, horror, AI artifacts
```

---

## Layer Prompts

### 1. Foreground Prompt

```text
Design the foreground layer for a 2.5D side-scrolling fantasy scene set in {BIOME}. The foreground must create depth, framing, and visual interest without blocking gameplay readability.

Use large near-camera shapes, strong silhouettes, and painterly hand-painted 3D surfaces. Include foreground elements appropriate to the biome such as oversized rocks, roots, grass tufts, hanging vines, broken fences, lanterns, tree trunks, stone edges, wooden posts, or decorative props.

The foreground should feel dimensional and atmospheric, but still light enough to support traversal and player movement. Keep shapes clear, elegant, and visually layered. Materials should match the project style: stylized, slightly flattened, and storybook-like.

The scene should support a side-view platformer camera and naturally frame the midground action.
```

### 2. Midground Prompt

```text
Design the midground layer for a 2.5D side-scrolling fantasy scene set in {BIOME}. This is the main playable space and should contain the clearest path, platforms, structures, and interactive elements.

Include the core environment features such as walkable ground, platforms, steps, bridges, ruined walls, fences, trees, props, and level-defining architecture. The midground should be visually rich but organized, with clear traversal logic and strong silhouette readability.

Use hand-painted 3D surfaces with a fairytale fantasy look inspired by Heroes of Might and Magic II. Keep the mood {MOOD}, the time of day {TIME_OF_DAY}, and the season {SEASON}. The scene should feel alive, layered, and handcrafted, while still supporting gameplay clarity.

Avoid overcrowding. Make the environment feel like a real level space with believable depth and natural composition.
```

### 3. Background Prompt

```text
Design the background layer for a 2.5D side-scrolling fantasy scene set in {BIOME}. The background should provide distance, mood, and world scale without competing with the playable space.

Use large scenic shapes only: distant mountains, treelines, sky gradients, far-off buildings, silhouettes, cliffs, clouds, mist, smoke, or horizon forms appropriate to the biome. Keep the background simplified, atmospheric, and painterly, with soft depth separation and subtle detail.

The background must support the fairytale fantasy direction of the project and reinforce the HoMM II-inspired world. It should feel like a living painted backdrop with layered atmosphere, but remain clearly non-interactive.

Avoid busy detail, harsh contrast, or anything that pulls attention away from the midground.
```

### 4. Wide Shot Prompt

```text
Create a wide establishing shot of a 2.5D side-scrolling fantasy level set in {BIOME}. Show foreground, midground, and background together in one cohesive composition.

The playable route should be clearly readable through the center of the image, with foreground framing elements near the camera, a strong midground path or traversal space, and a distant atmospheric background. The scene should demonstrate depth layering, level structure, and environmental storytelling.

Use the visual language of bright fairytale fantasy inspired by Heroes of Might and Magic II, with hand-painted 3D surfaces, storybook illustration appeal, and a slightly flattened painterly look. Use Ori and the Blind Forest for layered staging and camera depth, and Inside for functional background depth.

Keep the scene elegant, cohesive, and production-ready. The image should feel like a level overview for a fantasy platformer, not a cinematic poster.
```

---

## Environment Prompt

```text
Create a modular fantasy environment prop set for a 2.5D side-scrolling game set in {BIOME}. Include natural and man-made elements such as stones, rocks, trees, shrubs, grass, roots, barrels, crates, fences, ropes, signposts, lamps, wooden structures, village props, city props, ruins, or decorative objects depending on the biome.

The props should feel reusable, coherent, and designed for game production. Use stylized hand-painted 3D surfaces with a bright fairytale fantasy look inspired by Heroes of Might and Magic II. Keep shapes readable, slightly exaggerated, and visually harmonious.

The result should look like a cohesive environment kit for building levels, with strong material consistency and charming storybook personality.
```

---

## Creature Concept Prompt

```text
Create a creature concept sheet for an original fantasy creature belonging to {BIOME} in a {PROJECT_STYLE} world. The creature should be inspired by the fairytale fantasy tone of Heroes of Might and Magic II, but must be fully original and suitable for a 2.5D side-scrolling platformer.

Present the creature as a structured reference sheet with front view, side view, back view, face close-up, limb or claw detail, material close-ups, and expression notes. Include callouts for important anatomy and surface materials. The design should have a clear silhouette, believable fantasy anatomy, and a hand-painted 3D appearance that feels slightly flattened and illustrative.

Use materials and details that fit the creature’s nature, such as stone, wood, bark, moss, fur, bone, cloth, leaves, metal, crystals, or magical accents. Make the design visually memorable, readable, and production-friendly.
```

---

## Creature Prompt

```text
Create a fully rendered fantasy creature for a 2.5D side-scrolling platformer, set in {BIOME}. The creature should belong naturally to a bright fairytale fantasy world inspired by Heroes of Might and Magic II, with a hand-painted 3D look and a slightly flattened storybook rendering style.

Show the creature in a clear readable pose with strong silhouette, stylized anatomy, and cohesive fantasy materials. The design should feel alive, magical, and production-ready, with enough detail to look convincing in-game while remaining elegant and iconic.

This is not a concept sheet. It should be a finished in-world creature illustration or asset render that fits naturally into the side-view game environment.
```

---

## City Dwellers Prompt

```text
Create a set of city dwellers or village residents for a bright fairytale fantasy world. The characters should be mostly human, with anthropomorphic variants only when explicitly appropriate.

Use stylized proportions, expressive faces, practical fantasy clothing, and hand-painted 3D character rendering. Show a range of ages, jobs, and silhouettes while keeping the design cohesive with the Heroes of Might and Dream II inspired fairytale fantasy direction.

The characters should feel believable in a storybook city or village, with charm, personality, and clear readability for a 2.5D side-scrolling game. Keep the visual tone warm, whimsical, and grounded in fantasy life.
```

---

## Texture Prompt

```text
Create a seamless tileable PBR texture for {MATERIAL} in a bright fairytale fantasy world. The texture should be suitable for hand-painted 3D assets in a 2.5D side-scrolling platformer.

Generate a clean, production-ready material with stylized but physically believable surface detail. Use neutral lighting, no strong directional shadows, no perspective distortion, and no embedded objects unless they are part of the actual material. The surface must tile perfectly and remain visually consistent across a game environment.

The texture should match the Heroes of Might and Magic II inspired art direction: elegant, slightly painterly, rich in color harmony, and handcrafted. If supported, include albedo, normal, roughness, ambient occlusion, and height information, or otherwise present a clean material plate with clear surface variation.
```

---