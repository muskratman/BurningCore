Image Prompt System for a 2.5D Fantasy Platformer

Project Vision

This project is a 2.5D side-scrolling platformer set in a mature, stylized 3D fantasy world. The environment is designed as a functional in-game level view rather than a cinematic illustration. The visual direction is inspired by the atmosphere of Heroes of Might and Magic II, interpreted as high-fidelity hand-painted 3D surfaces, not pixel art and not cartoon animation.  

The scene must feel like a direct gameplay screencap with an orthographic-style side-view perspective. It must strictly avoid cute cartoon proportions, rubbery forms, and anything resembling a children’s animation.  

For composition and depth:
Ori and the Blind Forest for ultra-wide layered composition and atmospheric scale.  
Inside for functional level staging, readable playable space, and structural depth.  

Core Style Rules

Mature Fantasy Atmosphere: Elegant, majestic, and handcrafted.  
Direct Gameplay Screencap: Focus on asset rendering and level readability.  
Ultra-Wide Framing: Extreme horizontal field of view to show level segments.  
Orthographic Side-View: Camera level with horizon, 10-degree downward tilt. 
High-Fidelity Surfaces: Matte hand-painted 3D materials (not plastic or glossy).  
Strict Exclusions: No cartoon, no cute, no chibi, no settlement props in nature.  

Global Variables

{PROJECT_STYLE} = mature fantasy, HoMM II inspired, hand-painted 3D assets, elegant and atmospheric
{BIOME} = forest / swamp / mountains / snowlands / burning forest / village / city
{BIOME_SPECIFICS} = (e.g., layered stone, jagged cliffs, alpine flora, mist)
{BIOME_EXCLUSIONS} = (e.g., no houses, no roofs, no fences, no signs, no roads)
{MOOD} = calm / majestic / mysterious / slightly cold / dangerous
{TIME_OF_DAY} = morning / noon / dusk / night
{SEASON} = spring / summer / autumn / winter


Master Prompt

Direct gameplay screencap of a 2.5D side-scrolling platformer level. Create a stylized 3D fantasy environment designed as a functional in-game level mockup with an orthographic-style side-view perspective. The visual direction is mature fantasy, inspired by the atmospheric hand-painted 3D surfaces of Heroes of Might and Magic II, but rendered as high-fidelity 3D assets. Strictly avoid all cartoon aesthetics, cute proportions, and soft animation-style forms.
Use Ori and the Blind Forest as a reference for layered side-view composition and Inside as a reference for functional level staging. The camera must feel aligned with platformer gameplay. 
Use {BIOME} as the main setting. Focus on {BIOME_SPECIFICS}. Keep the mood {MOOD}, {TIME_OF_DAY}, and {SEASON}. {BIOME_EXCLUSIONS}.
Focus on clean silhouettes, believable fantasy scale, stylized proportions, and natural material variation. The result should look like a real side-scrolling game level, not a movie scene or cartoon illustration.


Negative Prompts

1. Universal Negative Prompt
(concept art sketch:1.2), (watercolor:1.2), oil painting, cartoon, animated film still, cute style, chibi, exaggerated facial expressions, thick outlines, rubbery shapes, childish proportions, pixel art, photorealism, low poly, anime, sci-fi, futuristic objects, modern objects, UI, watermark, logo, random text, messy composition, clutter, harsh directional shadows, cinematic lighting, lens flare, extreme depth of field, blurry image, overexposed highlights, neon colors, plastic look, broken anatomy, extra limbs, inconsistent perspective, cropped subject, noisy background, AI artifacts

2. Natural Biome Exclusions (For nature settings)
houses, roofs, buildings, villages, towns, cities, fences, windows, doors, chimneys, lamps, signs, roads, settlement props, urban elements


Layer Prompts

1. Wide Shot Prompt (Establishing Composition)
Create an ultra-wide panoramic establishing composition of a 2.5D side-scrolling fantasy level set in {BIOME}. Show foreground, midground, and background together in one cohesive scene.
Use an extreme horizontal framing with a long-distance camera placement. The camera must be almost level with the horizon, with only a slight downward tilt of about 10 degrees, creating a broad sense of space.
The playable path must be clearly readable as a long, unobstructed horizontal midground plane running through the center of the image. Foreground framing elements must be pushed to the extreme screen edges. The background shows distant atmospheric depth and world scale.
The image should feel like a functional level overview for fantasy platformer gameplay.

2. Foreground Prompt
Design the foreground layer for a 2.5D side-scrolling fantasy scene in {BIOME}. 
Foreground objects must be pushed to the extreme bottom and side edges of the frame. The center and mid-section of the image must remain completely empty to allow clear visibility for the gameplay area.
Use large near-camera shapes (e.g., {BIOME_SPECIFICS}) and strong silhouettes. Materials should be matte, hand-painted 3D surfaces. The foreground should frame the action without any visual clutter in the playable path.

3. Midground Prompt
Design the playable midground layer for a 2.5D side-scrolling fantasy scene in {BIOME}. 
This is the primary gameplay space. Establish a clear, long horizontal traversal plane or path running through the center of the image. Focus on functional platforms, ledges, and terrain (e.g., {BIOME_SPECIFICS}) designed for character movement.
Use high-fidelity asset rendering with a mature fantasy look. Keep silhouettes sharp and the traversal logic obvious. Avoid overcrowding; prioritize gameplay clarity.

4. Background Prompt
Design the far background layer for a 2.5D side-scrolling fantasy scene in {BIOME}. 
Compose this as a majestic panoramic backdrop with large distant forms: massive silhouettes, mountain ranges, or vast treelines appropriate to {BIOME}. Use soft atmospheric perspective and haze to create immense scale.
The camera remains level with the horizon (10-degree tilt). The background must be clearly non-interactive and visually pushed behind the midground to emphasize depth without distracting from the gameplay area.


Asset & Creature Prompts

1. Environment Asset Prompt
Create a modular fantasy environment prop set for {BIOME}. Include elements strictly appropriate to {BIOME_SPECIFICS}. {BIOME_EXCLUSIONS}.
The assets should feel reusable and designed for game production. Use stylized hand-painted 3D surfaces with a mature, classic fantasy look. Keep shapes sharp, silhouettes clean, and materials physically believable but stylized. Avoid all cute or rubbery proportions.

2. Creature Prompt
Create a fully rendered fantasy creature for a 2.5D platformer set in {BIOME}. The creature must belong to a mature fantasy world, featuring high-fidelity hand-painted 3D surfaces.
Show the creature in a clear side-view or 3/4 pose with a strong silhouette and believable anatomy. Avoid cartoonish or cute traits. The design should look like a production-ready game asset that fits naturally into a 2.5D level environment.


Prompt Assembly Recommendation
1. [Master Prompt] (Fill {BIOME}, {SPECIFICS}, {EXCLUSIONS}).  
2. [Wide Shot Prompt] (For the full level view).  
3. [Universal Negative Prompt] + [Natural Biome Exclusions].






