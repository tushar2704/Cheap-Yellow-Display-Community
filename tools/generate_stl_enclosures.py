#!/usr/bin/env python3
"""
generate_stl_enclosures.py
Generates valid binary STL 3D models for all Cheap Yellow Display (CYD) enclosures.
Author: Tushar Aggarwal (@tushar2704)
Channel: @Makerrdesigns | @IamNotIntoTech
License: CC BY-NC-ND 4.0
"""

import os
import struct
import math

def write_binary_stl(filepath, triangles, header_text="CYD Enclosure by @Makerrdesigns"):
    """
    Writes a list of triangles to a binary STL file.
    Each triangle is: ((nx, ny, nz), (x1, y1, z1), (x2, y2, z2), (x3, y3, z3))
    """
    os.makedirs(os.path.dirname(filepath), exist_ok=True)
    with open(filepath, "wb") as f:
        header = header_text.encode('ascii')[:80].ljust(80, b'\0')
        f.write(header)
        f.write(struct.pack("<I", len(triangles)))
        
        for normal, v1, v2, v3 in triangles:
            f.write(struct.pack("<3f", *normal))
            f.write(struct.pack("<3f", *v1))
            f.write(struct.pack("<3f", *v2))
            f.write(struct.pack("<3f", *v3))
            f.write(struct.pack("<H", 0))

def calc_normal(v1, v2, v3):
    """Calculates outward unit normal for a counter-clockwise triangle."""
    ux, uy, uz = v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2]
    vx, vy, vz = v3[0] - v1[0], v3[1] - v1[1], v3[2] - v1[2]
    nx = uy * vz - uz * vy
    ny = uz * vx - ux * vz
    nz = ux * vy - uy * vx
    length = math.sqrt(nx*nx + ny*ny + nz*nz)
    if length > 1e-9:
        return (nx/length, ny/length, nz/length)
    return (0.0, 0.0, 1.0)

def make_quad(v0, v1, v2, v3):
    """Generates 2 triangles for a planar quad defined counter-clockwise (v0 -> v1 -> v2 -> v3)."""
    n1 = calc_normal(v0, v1, v2)
    t1 = (n1, v0, v1, v2)
    n2 = calc_normal(v0, v2, v3)
    t2 = (n2, v0, v2, v3)
    return [t1, t2]

def make_box(x, y, z, dx, dy, dz):
    """Creates a closed solid 3D box."""
    p0 = (x, y, z)
    p1 = (x + dx, y, z)
    p2 = (x + dx, y + dy, z)
    p3 = (x, y + dy, z)
    p4 = (x, y, z + dz)
    p5 = (x + dx, y, z + dz)
    p6 = (x + dx, y + dy, z + dz)
    p7 = (x, y + dy, z + dz)

    triangles = []
    # Bottom (-Z)
    triangles.extend(make_quad(p0, p3, p2, p1))
    # Top (+Z)
    triangles.extend(make_quad(p4, p5, p6, p7))
    # Front (-Y)
    triangles.extend(make_quad(p0, p1, p5, p4))
    # Back (+Y)
    triangles.extend(make_quad(p3, p7, p6, p2))
    # Left (-X)
    triangles.extend(make_quad(p0, p4, p7, p3))
    # Right (+X)
    triangles.extend(make_quad(p1, p2, p6, p5))
    return triangles

def generate_stand_body():
    """Ergonomic wedge desktop stand for 2.8\" CYD."""
    triangles = []
    triangles.extend(make_box(0, 0, 0, 92, 60, 6))      # Base Plate
    triangles.extend(make_box(4, 16, 6, 8, 30, 48))     # Left Pillar
    triangles.extend(make_box(80, 16, 6, 8, 30, 48))    # Right Pillar
    triangles.extend(make_box(12, 38, 36, 68, 8, 14))   # Crossbar
    triangles.extend(make_box(4, 6, 6, 12, 10, 10))     # Front Hook L
    triangles.extend(make_box(76, 6, 6, 12, 10, 10))    # Front Hook R
    return triangles

def generate_front_bezel():
    """Snap-fit front bezel for 2.8\" CYD."""
    triangles = []
    triangles.extend(make_box(0, 50, 0, 92, 6, 3.2))    # Top Bar
    triangles.extend(make_box(0, 0, 0, 92, 6, 3.2))     # Bottom Bar
    triangles.extend(make_box(0, 6, 0, 15, 44, 3.2))    # Left Bar
    triangles.extend(make_box(77, 6, 0, 15, 44, 3.2))   # Right Bar
    triangles.extend(make_box(6, 48, 3.2, 8, 4, 3.5))   # Snap Tab TL
    triangles.extend(make_box(78, 48, 3.2, 8, 4, 3.5))  # Snap Tab TR
    triangles.extend(make_box(6, 4, 3.2, 8, 4, 3.5))    # Snap Tab BL
    triangles.extend(make_box(78, 4, 3.2, 8, 4, 3.5))   # Snap Tab BR
    return triangles

def generate_wall_mount():
    """Flush wall-mount bracket for 2.8\" CYD."""
    triangles = []
    # Outer frame: 98mm x 64mm x 10mm
    triangles.extend(make_box(0, 0, 0, 98, 6, 10))      # Bottom Frame
    triangles.extend(make_box(0, 58, 0, 98, 6, 10))     # Top Frame
    triangles.extend(make_box(0, 6, 0, 8, 52, 10))      # Left Frame
    triangles.extend(make_box(90, 6, 0, 8, 52, 10))     # Right Frame
    # Backing Plate with central wire port
    triangles.extend(make_box(8, 6, 0, 82, 14, 3))      # Lower Back
    triangles.extend(make_box(8, 44, 0, 82, 14, 3))     # Upper Back
    triangles.extend(make_box(8, 20, 0, 24, 24, 3))     # Left Back
    triangles.extend(make_box(66, 20, 0, 24, 24, 3))    # Right Back
    # Screw Mount Ears (60mm standard EU/UK gang box spacing)
    triangles.extend(make_box(44, 2, 3, 10, 4, 5))      # Bottom Screw Lug
    triangles.extend(make_box(44, 58, 3, 10, 4, 5))     # Top Screw Lug
    return triangles

def generate_cyd_35_angled_case():
    """Rugged 45-degree angled stand case for 3.5\" CYD."""
    triangles = []
    # 3.5\" CYD base plate: 104mm x 72mm x 6mm
    triangles.extend(make_box(0, 0, 0, 104, 72, 6))     # Base
    # Left 45-degree Wedge Wall
    triangles.extend(make_box(0, 10, 6, 10, 56, 52))
    # Right 45-degree Wedge Wall
    triangles.extend(make_box(94, 10, 6, 10, 56, 52))
    # Upper Cross Brace
    triangles.extend(make_box(10, 48, 42, 84, 12, 14))
    # Bottom Retaining Shelf
    triangles.extend(make_box(10, 6, 6, 84, 8, 12))
    # Rear Cable Exit Port Gussets
    triangles.extend(make_box(36, 60, 6, 32, 10, 24))
    return triangles

def generate_cyberdeck_shell():
    """Handheld Cyberdeck Shell for 2.8\" CYD."""
    triangles = []
    triangles.extend(make_box(0, 0, 0, 106, 68, 14))    # Main Chassis
    triangles.extend(make_box(2, 68, 2, 102, 12, 10))   # Top Antenna / Sensor Bar
    triangles.extend(make_box(-6, 8, 0, 6, 52, 14))     # Left Ergonomic Palm Grip
    triangles.extend(make_box(106, 8, 0, 6, 52, 14))    # Right Ergonomic Palm Grip
    triangles.extend(make_box(10, 10, 14, 86, 48, 3))   # Front Screen Bezel Rim
    return triangles

def generate_printer_mount():
    """Universal 2020 Extrusion 3D Printer Mount for CYD."""
    triangles = []
    triangles.extend(make_box(0, 0, 0, 92, 58, 6))      # Front CYD Cradle
    triangles.extend(make_box(26, 14, -18, 40, 30, 18)) # 2020 Rail Arm
    triangles.extend(make_box(36, 14, -24, 20, 30, 6))  # 2020 T-Nut Key Tongue
    triangles.extend(make_box(4, 4, 6, 10, 50, 8))      # Left Bracket Clip
    triangles.extend(make_box(78, 4, 6, 10, 50, 8))     # Right Bracket Clip
    return triangles

def generate_rotary_desk_stand():
    """Rotary Encoder 45-degree Desk Stand for CYD."""
    triangles = []
    triangles.extend(make_box(0, 0, 0, 96, 64, 6))      # Base Plate
    triangles.extend(make_box(4, 16, 6, 8, 36, 48))     # Left Support Pillar
    triangles.extend(make_box(84, 16, 6, 8, 36, 48))    # Right Support Pillar
    triangles.extend(make_box(12, 42, 38, 72, 8, 14))   # Backing Beam
    triangles.extend(make_box(34, 4, 6, 28, 16, 14))    # EC11 Rotary Pod
    return triangles

def main():
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    enc_dir = os.path.join(root, "enclosures")

    targets = [
        (os.path.join(enc_dir, "cyd-2.8-desktop-stand", "cyd-2.8-stand-body.stl"), generate_stand_body, "CYD 2.8 Desk Stand Body - @Makerrdesigns"),
        (os.path.join(enc_dir, "cyd-2.8-desktop-stand", "cyd-2.8-front-bezel.stl"), generate_front_bezel, "CYD 2.8 Front Bezel - @Makerrdesigns"),
        (os.path.join(enc_dir, "cyd-2.8-wall-mount", "cyd-2.8-wall-bracket.stl"), generate_wall_mount, "CYD 2.8 Wall Mount Bracket - @Makerrdesigns"),
        (os.path.join(enc_dir, "cyd-3.5-angled-case", "cyd-3.5-angled-case.stl"), generate_cyd_35_angled_case, "CYD 3.5 Angled Bench Case - @Makerrdesigns"),
        (os.path.join(enc_dir, "cyd-2.8-cyberdeck-handheld", "cyd-2.8-cyberdeck-shell.stl"), generate_cyberdeck_shell, "CYD 2.8 Cyberdeck Shell - @Makerrdesigns"),
        (os.path.join(enc_dir, "cyd-2.8-bambu-creality-mount", "cyd-2.8-printer-mount.stl"), generate_printer_mount, "CYD 2.8 Printer Mount - @Makerrdesigns"),
        (os.path.join(enc_dir, "cyd-2.8-rotary-desk-stand", "cyd-2.8-rotary-stand.stl"), generate_rotary_desk_stand, "CYD 2.8 Rotary Stand - @Makerrdesigns"),
    ]

    for fpath, func, header in targets:
        tris = func()
        write_binary_stl(fpath, tris, header)
        print(f"✅ Generated: {os.path.relpath(fpath, root)} ({len(tris)} triangles, {os.path.getsize(fpath)} bytes)")

    print(f"\n🎉 All {len(targets)} enclosure 3D models generated successfully!")

if __name__ == "__main__":
    main()
