#!/usr/bin/env python3
"""
pixil_to_bumpcsv_1to1.py
Convert a Pixilart .pixil file to your bump CSV format with a 1:1 pixel-to-bump mapping.

Output format:
MAX_CURRENT = <...>
SERIES_RESISTANCE = <...>
SERIES_INDUCTANCE = <...>
SHUNT_CAPACITANCE = <...>
BEGIN_CHIPLET <NAME> <ROWS> <COLS>
A01,<LABEL>
...
END_CHIPLET

Usage:
  python pixil_to_bumpcsv_1to1.py input.pixil output.csv --name L2 \
      --map "(168,230,29)=POWER_3,(0,183,239)=GROUND,(0,0,0)=NC"

Notes:
- Composites ALL layers of the first frame in order (top over bottom).
- If --map is omitted, a heuristic mapping is used:
    * Black (0,0,0,*) -> NC
    * The most common non-black opaque color -> POWER_3
    * The second most common non-black opaque color -> GROUND
    * Anything else -> SIG
"""

import argparse, base64, io, json, re, string, sys
from typing import Dict, Tuple, List
from PIL import Image
import numpy as np

RGBA = Tuple[int,int,int,int]

def excel_letters(n: int) -> str:
    """1->A, 2->B, ..., 26->Z, 27->AA, 28->AB, ..."""
    s = ""
    while n > 0:
        n, r = divmod(n - 1, 26)
        s = chr(65 + r) + s
    return s

def parse_color_map(s: str) -> Dict[RGBA, str]:
    """
    Parse a color->label map string like:
      "(168,230,29)=POWER_3,(0,183,239)=GROUND,(0,0,0)=NC"
    or using semicolons:
      "(168,230,29)=POWER_3;(0,183,239)=GROUND;(0,0,0)=NC"
    Handles commas inside the parentheses without breaking.
    Accepts (r,g,b) or (r,g,b,a). Alpha defaults to 255.
    """
    if not s:
        return {}

    parts = []
    buf = []
    depth = 0
    for ch in s:
        if ch == '(':
            depth += 1
        elif ch == ')':
            depth = max(0, depth - 1)
        # Split only on separators at top level
        if depth == 0 and ch in ',;':
            part = ''.join(buf).strip()
            if part:
                parts.append(part)
            buf = []
        else:
            buf.append(ch)
    last = ''.join(buf).strip()
    if last:
        parts.append(last)

    out: Dict[RGBA, str] = {}
    for part in parts:
        if not part:
            continue
        # split on the first '=' only
        if '=' not in part:
            raise ValueError(f"Bad mapping segment (missing '='): {part!r}")
        key_str, label = part.split('=', 1)
        label = label.strip()
        key_str = key_str.strip().strip('()').strip()
        if not key_str:
            raise ValueError(f"Empty color tuple in segment: {part!r}")
        nums = tuple(int(x.strip()) for x in key_str.split(','))
        if len(nums) == 3:
            nums = nums + (255,)
        if len(nums) != 4:
            raise ValueError(f"Color must be (r,g,b) or (r,g,b,a): {part!r}")
        out[nums] = label
    return out

def load_pixil_first_frame_rgba(pixil_path: str) -> Image.Image:
    """Load first frame, composite all layers into a single RGBA image."""
    with open(pixil_path, "r", encoding="utf-8") as f:
        pix = json.load(f)
    frames = pix.get("frames", [])
    if not frames:
        raise SystemExit("No frames in .pixil")
    layers = frames[0].get("layers", [])
    if not layers:
        raise SystemExit("No layers in first frame")

    base_img = None
    for layer in layers:
        src = layer.get("src")
        if not isinstance(src, str) or "," not in src:
            continue
        b64 = src[src.rfind(",")+1:]
        try:
            layer_img = Image.open(io.BytesIO(base64.b64decode(b64))).convert("RGBA")
        except Exception as e:
            raise SystemExit(f"Failed to decode a layer image: {e}")
        if base_img is None:
            base_img = Image.new("RGBA", layer_img.size, (0,0,0,0))
        base_img.alpha_composite(layer_img)
    if base_img is None:
        raise SystemExit("Could not compose any layer images from the first frame.")
    return base_img

def build_heuristic_map(arr: np.ndarray) -> Dict[RGBA,str]:
    pixels = arr.reshape(-1,4)
    opaque = pixels[pixels[:,3] > 0]
    if opaque.size == 0:
        return {}
    colors, counts = np.unique(opaque, axis=0, return_counts=True)
    order = np.argsort(-counts)
    sorted_colors: List[RGBA] = [tuple(int(x) for x in colors[i]) for i in order]

    cmap: Dict[RGBA,str] = {}
    # Black -> NC
    for c in sorted_colors:
        r,g,b,a = c
        if (r,g,b) == (0,0,0):
            cmap[c] = "NC"
    # Two most common non-black -> POWER_3, GROUND
    nonblack = [c for c in sorted_colors if c[:3] != (0,0,0)]
    if nonblack:
        cmap[nonblack[0]] = "POWER_3"
    if len(nonblack) > 1:
        cmap[nonblack[1]] = "GROUND"
    # All others -> SIG, unless already assigned
    for c in sorted_colors:
        if c not in cmap:
            cmap[c] = "SIG"
    return cmap

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("input_pixil")
    ap.add_argument("output_csv")
    ap.add_argument("--name", default="L2", help="Chiplet name for BEGIN_CHIPLET line")
    ap.add_argument("--map", dest="mapstr", default=None, help="Explicit color map '(r,g,b)=LABEL,(r,g,b)=LABEL2'")
    # Header fields
    ap.add_argument("--imax", default="0.021 A")
    ap.add_argument("--rs", default="1000 mOhm")
    ap.add_argument("--ls", default="75 nH")
    ap.add_argument("--cs", default="5 pF")
    # Label to use for fully transparent pixels
    ap.add_argument("--transparent", default="NC")
    args = ap.parse_args()

    rgba = load_pixil_first_frame_rgba(args.input_pixil)
    arr = np.array(rgba)  # H x W x 4
    H, W, _ = arr.shape

    # Color mapping
    cmap = parse_color_map(args.mapstr) if args.mapstr else build_heuristic_map(arr)

    # Emit CSV
    lines = [
        f"MAX_CURRENT = {args.imax}",
        f"SERIES_RESISTANCE = {args.rs}",
        f"SERIES_INDUCTANCE = {args.ls}",
        f"SHUNT_CAPACITANCE = {args.cs}",
        f"BEGIN_CHIPLET {args.name} {H} {W}",
    ]

    # Column number padding (at least 2 digits, scales with width)
    pad = max(2, len(str(W)))

    for r in range(H):
        row_name = excel_letters(r+1)  # 0->A
        for c in range(W):
            R,G,B,A = map(int, arr[r, c])
            key = (R,G,B,A)
            label = (args.transparent if A == 0 else cmap.get(key, "SIG"))
            lines.append(f"{row_name}{c+1:0{pad}d},{label}")

    lines.append("END_CHIPLET")

    with open(args.output_csv, "w", encoding="ascii") as f:
        f.write("\n".join(lines))

    # Print a small legend to stdout
    print(f"Image size: {H} x {W}")
    print("Color legend (RGBA -> LABEL):")
    # Deduplicate while preserving first label
    seen = set()
    for k, v in cmap.items():
        if k not in seen:
            print(f"  {k} -> {v}")
            seen.add(k)

if __name__ == "__main__":
    main()
