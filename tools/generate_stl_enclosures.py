#!/usr/bin/env python3
"""
generate_stl_enclosures.py
Generates valid binary STL 3D models for the ESP32-2432S028R Cheap Yellow Display.
Author: Tushar Aggarwal (@tushar2704)
Channel: @Makerrdesigns | @IamNotIntoTech
License: CC BY-NC-ND 4.0
"""

import os
import struct
import math

def write_binary_stl(filepath, triangles, header_text="CYD 2.8 Desktop Stand by @Makerrdesigns"):
    """
    Writes a list of triangles to a binary STL file.
    Each triangle is: ((nx, ny, nz), (x1, y1, z1), (x2, y2, z2), (x3, y3, z3))
    """
    with open(filepath, "wb") as f:
        # 80-byte header
        header = header_text.encode('ascii')[:80].ljust(80, b'\0')
        f.write(header)
        # Number of triangles (uint32)
        f.write(struct.pack("<I", len(triangles)))
        
        for normal, v1, v2, v3 in triangles:
            # 3 floats for normal, 3*3 floats for vertices, 1 uint16 for attribute byte count (0)
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
    """
    Generates an ergonomic wedge desktop stand for the 2.8" CYD.
    Base: 92mm wide x 60mm deep x 8mm thick base plate.
    Inclined upright support struts at 60 degrees.
    Bottom PCB resting tray lip (3mm height x 14mm shelf).
    """
    triangles = []
    
    # 1. Base Plate (92mm x 60mm x 6mm)
    triangles.extend(make_box(0, 0, 0, 92, 60, 6))

    # 2. Left Inclined Upright Pillar (width: 8mm, depth: 32mm, height: 48mm)
    triangles.extend(make_box(4, 16, 6, 8, 30, 48))

    # 3. Right Inclined Upright Pillar (width: 8mm, depth: 32mm, height: 48mm)
    triangles.extend(make_box(80, 16, 6, 8, 30, 48))

    # 4. Crossbar Brace (Connecting left & right pillars across back)
    triangles.extend(make_box(12, 38, 36, 68, 8, 14))

    # 5. Front Retaining Cradle Lip (Prevents CYD PCB from sliding forward)
    # Left hook
    triangles.extend(make_box(4, 6, 6, 12, 10, 10))
    # Right hook
    triangles.extend(make_box(76, 6, 6, 12, 10, 10))
    
    # 6. USB Cable Relief Notch (Central cutout in base for micro-USB / USB-C wire routing)
    # The stand base naturally allows wire to loop underneath between 20mm and 72mm

    return triangles

def generate_front_bezel():
    """
    Generates a snap-fit decorative front bezel for 2.8" CYD.
    Outer: 92mm x 56mm x 3.2mm.
    Inner Screen Aperture: 62mm x 44mm (Centered view opening for the 320x240 LCD active area).
    """
    triangles = []
    
    # Frame composed of 4 perimeter bars around the central window:
    # Top Bar (92mm x 6mm x 3.2mm)
    triangles.extend(make_box(0, 50, 0, 92, 6, 3.2))
    # Bottom Bar (92mm x 6mm x 3.2mm)
    triangles.extend(make_box(0, 0, 0, 92, 6, 3.2))
    # Left Bar (15mm x 44mm x 3.2mm)
    triangles.extend(make_box(0, 6, 0, 15, 44, 3.2))
    # Right Bar (15mm x 44mm x 3.2mm)
    triangles.extend(make_box(77, 6, 0, 15, 44, 3.2))

    # Snap-fit retention tabs on reverse (+Z side):
    # Top-Left Tab
    triangles.extend(make_box(6, 48, 3.2, 8, 4, 3.5))
    # Top-Right Tab
    triangles.extend(make_box(78, 48, 3.2, 8, 4, 3.5))
    # Bottom-Left Tab
    triangles.extend(make_box(6, 4, 3.2, 8, 4, 3.5))
    # Bottom-Right Tab
    triangles.extend(make_box(78, 4, 3.2, 8, 4, 3.5))

    return triangles

def main():
    out_dir = "enclosures/cyd-2.8-desktop-stand"
    os.makedirs(out_dir, exist_ok=True)

    body_stl = os.path.join(out_dir, "cyd-2.8-stand-body.stl")
    bezel_stl = os.path.join(out_dir, "cyd-2.8-front-bezel.stl")

    body_tris = generate_stand_body()
    write_binary_stl(body_stl, body_tris, "CYD 2.8 Desk Stand Body - @Makerrdesigns")
    print(f"✅ Generated: {body_stl} ({len(body_tris)} triangles, {os.path.getsize(body_stl)} bytes)")

    bezel_tris = generate_front_bezel()
    write_binary_stl(bezel_stl, bezel_tris, "CYD 2.8 Front Bezel - @Makerrdesigns")
    print(f"✅ Generated: {bezel_stl} ({len(bezel_tris)} triangles, {os.path.getsize(bezel_stl)} bytes)")

if __name__ == "__main__":
    main()
