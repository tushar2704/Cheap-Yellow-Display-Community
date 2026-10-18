# 🖨️ 3D Printing & Slicing Master Guide — CYD Enclosures

> Pro-tips, slicer profiles, material selection, and tolerance guidelines for 3D printing Cheap Yellow Display cases, stands, and bezels. Designed by [@Makerrdesigns](https://youtube.com/@Makerrdesigns).

---

## 🧭 Recommended Slicer Settings

| Setting | Recommended Value | Notes |
|---|---|---|
| **Layer Height** | `0.20 mm` | Use `0.16 mm` for high-detail curved front bezels |
| **First Layer Height** | `0.20 mm` | Ensures strong bed adhesion |
| **Perimeters / Wall Loops** | `3 to 4 walls` | Crucial for snap-fit flexibility and screw thread grip |
| **Top / Bottom Solid Layers** | `4 top, 4 bottom` | Prevents pillowing over infill |
| **Infill Percentage** | `15% – 20%` | Sufficient for rigid desktop and wall cases |
| **Infill Pattern** | **Gyroid** or **Cubic** | Uniform strength in all X/Y/Z axes |
| **Print Speed** | Standard profile | Slow down to 40 mm/s on outer perimeters for clean surface finish |

---

## 🧵 Material Selection Guide

### 1. PLA / PLA+ (Best for Desktop Stands)
- **Pros**: Easy to print, crisp overhangs, zero warping, vibrant colors (including CYD Yellow!).
- **Cons**: Deforms in direct sunlight or hot car interiors (>50°C).
- **Use Case**: [Desktop Wedge Stand](cyd-2.8-desktop-stand/), desk clocks, weather stations.

### 2. PETG (Best for Smart Home Wall Mounts)
- **Pros**: High thermal resistance (up to 75°C), semi-flexible snaps that won't fatigue or snap off, UV resistant.
- **Cons**: Prone to fine stringing if filament is moist.
- **Use Case**: [Flush Wall Mounts](cyd-2.8-wall-mount/), workshop monitors.

### 3. ABS / ASA (Best for Heavy-Duty & Enclosed Builds)
- **Pros**: High temperature resistance (100°C), UV stable, can be vapor smoothed with acetone.
- **Cons**: Requires an enclosed 3D printer chamber.
- **Use Case**: 3D printer controller screens, automotive telemetry boxes.

---

## 🔧 Engineering Tolerances & Snap-Fit Assembly

All enclosures in this community repository are engineered with a **0.25 mm nominal clearance gap** between mating parts:
- **Loose Fit?**: If the front bezel feels loose, check your slicer's **Flow Rate / Extrusion Multiplier**. Under-extrusion shrinks snap teeth.
- **Too Tight?**: If parts are hard to snap together, check for **Elephant's Foot** on the first layer. In Bambu Studio / PrusaSlicer, enable `Elephant Foot Compensation: 0.15 mm`.
- **Bed Surface**: Print face-down on a **Textured PEI Sheet** to give the front bezel a premium, glare-free, matte-textured surface finish!

---

## 📺 Video Tutorials
- 🖨️ CAD Modeling & Slicing Walkthroughs: [@Makerrdesigns on YouTube](https://youtube.com/@Makerrdesigns)
- 🎬 Electronics & Code Tutorials: [@IamNotIntoTech on YouTube](https://youtube.com/@IamNotIntoTech)
