#!/usr/bin/env python3
"""
audit_repository.py — Complete Community Repository Verification Tool
Author: Tushar Aggarwal (tushar2704)
Repository: https://github.com/tushar2704/Cheap-Yellow-Display-Community

Verifies:
1. All flasher manifests point to valid, non-empty binaries.
2. All cards in flasher/index.html and cyd-flasher/index.html are resolvable.
3. Every project has valid code files, platformio.ini, and documentation.
4. Zero forbidden keywords exist in source trees.
"""

import os
import sys
import json
import re

REPO_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

def test_manifests():
    manifests_dir = os.path.join(REPO_DIR, "flasher", "manifests")
    binaries_dir = os.path.join(REPO_DIR, "flasher", "binaries")
    
    missing_files = []
    manifest_count = 0
    for fname in sorted(os.listdir(manifests_dir)):
        if not fname.endswith(".json"):
            continue
        manifest_count += 1
        fpath = os.path.join(manifests_dir, fname)
        with open(fpath, "r", encoding="utf-8") as f:
            try:
                data = json.load(f)
            except Exception as e:
                missing_files.append((fname, f"JSON parse error: {e}"))
                continue
        
        builds = data.get("builds", [])
        for build in builds:
            parts = build.get("parts", [])
            for part in parts:
                rel_path = part.get("path", "")
                bin_name = os.path.basename(rel_path)
                full_bin_path = os.path.join(binaries_dir, bin_name)
                if not os.path.exists(full_bin_path):
                    missing_files.append((fname, f"Missing binary: {bin_name}"))
                else:
                    sz = os.path.getsize(full_bin_path)
                    if sz == 0:
                        missing_files.append((fname, f"Zero byte binary: {bin_name}"))

    print(f"Audited {manifest_count} manifests. Errors: {len(missing_files)}")
    for m in missing_files:
        print(f"  [ERROR] {m[0]}: {m[1]}")
    return len(missing_files) == 0

def test_flasher_cards():
    flasher_html = os.path.join(REPO_DIR, "flasher", "index.html")
    with open(flasher_html, "r", encoding="utf-8") as f:
        content = f.read()
    
    manifest_matches = re.findall(r'manifest:\s*"([^"]+)"', content)
    bin_matches = re.findall(r'bin:\s*"([^"]+)"', content)
    
    missing = []
    for m in manifest_matches:
        full_p = os.path.join(REPO_DIR, "flasher", m)
        if not os.path.exists(full_p):
            missing.append(f"Flasher manifest not found: {m}")
            
    for b in bin_matches:
        full_p = os.path.join(REPO_DIR, "flasher", b)
        if not os.path.exists(full_p):
            missing.append(f"Flasher binary not found: {b}")

    print(f"Audited flasher/index.html cards: {len(manifest_matches)} manifests, {len(bin_matches)} binaries. Errors: {len(missing)}")
    for m in missing:
        print(f"  [ERROR] {m}")
    return len(missing) == 0

def test_cyd_flasher_cards():
    flasher_html = os.path.join(REPO_DIR, "cyd-flasher", "index.html")
    with open(flasher_html, "r", encoding="utf-8") as f:
        content = f.read()
    
    manifest_matches = re.findall(r"manifest:\s*'([^']+)'", content)
    missing = []
    manifests_dir = os.path.join(REPO_DIR, "flasher", "manifests")
    for m in manifest_matches:
        full_p = os.path.join(manifests_dir, m)
        if not os.path.exists(full_p):
            missing.append(f"cyd-flasher manifest not found: {m}")

    print(f"Audited cyd-flasher/index.html: {len(manifest_matches)} manifests. Errors: {len(missing)}")
    for m in missing:
        print(f"  [ERROR] {m}")
    return len(missing) == 0

def test_forbidden_words():
    forbidden = ["witness" + "menow"]
    found = []
    for root, dirs, files in os.walk(REPO_DIR):
        if ".git" in root or ".pio" in root or "node_modules" in root:
            continue
        for file in files:
            if file.endswith((".ino", ".cpp", ".h", ".md", ".json", ".html", ".py", ".yaml")):
                fpath = os.path.join(root, file)
                try:
                    with open(fpath, "r", encoding="utf-8", errors="ignore") as f:
                        text = f.read()
                        for word in forbidden:
                            if word.lower() in text.lower():
                                found.append((fpath, word))
                except Exception:
                    pass
    print(f"Audited forbidden words in repo. Found occurrences: {len(found)}")
    for f in found:
        print(f"  [FORBIDDEN] {f[0]}: contains '{f[1]}'")
    return len(found) == 0

def test_all_projects_structure():
    categories = ["beginner", "intermediate", "advanced", "capacitive"]
    total_projects = 0
    errors = []
    for cat in categories:
        cat_dir = os.path.join(REPO_DIR, "projects", cat)
        if not os.path.exists(cat_dir):
            continue
        for item in sorted(os.listdir(cat_dir)):
            pdir = os.path.join(cat_dir, item)
            if not os.path.isdir(pdir):
                continue
            total_projects += 1
            has_readme = os.path.exists(os.path.join(pdir, "README.md"))
            has_pio = os.path.exists(os.path.join(pdir, "platformio.ini"))
            
            code_files = [f for f in os.listdir(pdir) if f.endswith((".ino", ".cpp"))]
            src_dir = os.path.join(pdir, "src")
            if os.path.exists(src_dir) and os.path.isdir(src_dir):
                code_files += [f for f in os.listdir(src_dir) if f.endswith((".ino", ".cpp"))]
            
            if not has_readme:
                errors.append(f"{cat}/{item} missing README.md")
            if not has_pio:
                errors.append(f"{cat}/{item} missing platformio.ini")
            if not code_files:
                errors.append(f"{cat}/{item} has NO code files")

    print(f"Audited {total_projects} projects across 4 tiers. Errors: {len(errors)}")
    for e in errors:
        print(f"  [ERROR] {e}")
    return len(errors) == 0

if __name__ == "__main__":
    r1 = test_manifests()
    r2 = test_flasher_cards()
    r3 = test_cyd_flasher_cards()
    r4 = test_forbidden_words()
    r5 = test_all_projects_structure()
    
    if r1 and r2 and r3 and r4 and r5:
        print("\n>>> ALL TESTS PASSED SUCCESSFULLY! 100% HEALTHY! <<<")
        sys.exit(0)
    else:
        print("\n>>> AUDIT FAILED! SEE ERRORS ABOVE! <<<")
        sys.exit(1)
