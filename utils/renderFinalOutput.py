from __future__ import annotations
import re
import sys
import os
from dataclasses import dataclass
from typing import Dict, Tuple, List, Optional
import matplotlib.pyplot as plt
from argparse import ArgumentParser
from math import sqrt


# ----------------------------
# Console colors (ANSI)
# ----------------------------
COLORRST    = "\u001b[0m"
BLACK       = "\u001b[30m"
RED         = "\u001b[31m"
GREEN       = "\u001b[32m"
YELLOW      = "\u001b[33m"
BLUE        = "\u001b[34m"
MAGENTA     = "\u001b[35m"
CYAN        = "\u001b[36m"
WHITE       = "\u001b[37m"

# ----------------------------
# Predefined signal palette
# ----------------------------
SIGNAL_COLORS = {
    "CHIPLET": "#B8B8B8",
    "EMPTY": "none",
    "UNKNOWN": "none",
    "SIGNAL": "#B0B0B0",
    "POWER_1": "#1e81b0",
    "POWER_2": "#e67e22",
    "POWER_3": "#ffc107",
    "POWER_4": "#b29dd9",
    "POWER_5": "#fc83bc",
    "POWER_6": "#72f2ee",
    "POWER_7": "#c0392b",
    "POWER_8": "#21b2ab",
    "POWER_9": "#b0f294",
    "POWER_10": "#8d57a3",
    "GROUND": "#5cb85c",
    "OBSTACLE": "#663300"
}

@dataclass(frozen=True)
class NodeRef:
    x: int
    y: int

@dataclass
class NodeInfo:
    signal: str
    up: Optional[str]   # kept for compatibility with input format, but unused now
    down: Optional[str]

@dataclass
class Edge:
    x0: int
    y0: int
    x1: int
    y1: int
    signal: str

_HEADER_RE = re.compile(r"^\s*PHYSICAL_IMPLEMENTATION\s+VISUALISATION\s*$", re.I)
_DIM_RE    = re.compile(r"^\s*W\s*=\s*(\d+)\s*,\s*H\s*=\s*(\d+)\s*$")
# Allow trailing text after 'down = ...'
_NODE_RE   = re.compile(
    r"^\s*(\d+)\s*,\s*(\d+)\s*:\s*([^\s]+)\s+up\s*=\s*([^\s]+)\s+down\s*=\s*([^\s]+).*$",
    re.I,
)

def _norm_sig(s: Optional[str]) -> Optional[str]:
    if s is None: return None
    if s.lower() == "nullptr": return None
    return s

def _is_empty(sig: Optional[str]) -> bool:
    return sig is None or sig.lower() in ("empty", "unknown")

def parse_pdn_text(path: str, verbose: bool=False) -> Tuple[int,int, Dict[NodeRef, NodeInfo], List[Edge]]:
    if verbose:
        print(f"{CYAN}Reading:{COLORRST} {path}")
    with open(path, "r", encoding="utf-8") as f:
        lines = [ln.rstrip("\n") for ln in f]

    if not lines or not _HEADER_RE.match(lines[0]):
        raise ValueError("File does not start with 'PHYSICAL_IMPLEMENTATION VISUALISATION' header.")

    m = _DIM_RE.match(lines[1])
    if not m:
        raise ValueError("Second line must be 'W = <width>, H = <height>'.")
    W, H = int(m.group(1)), int(m.group(2))

    nodes: Dict[NodeRef, NodeInfo] = {}
    edges: List[Edge] = []

    # First, consume node lines (expect W*H of them)
    idx = 2
    consumed_nodes = 0
    while idx < len(lines) and consumed_nodes < W * H:
        ln = lines[idx].strip()
        idx += 1
        if not ln:
            continue
        m = _NODE_RE.match(ln)
        if not m:
            raise ValueError(f"Malformed node line at #{idx}: {ln!r}")
        i, j = int(m.group(1)), int(m.group(2))
        sig   = m.group(3)
        up    = _norm_sig(m.group(4))
        down  = _norm_sig(m.group(5))
        nodes[NodeRef(i, j)] = NodeInfo(signal=sig, up=up, down=down)
        consumed_nodes += 1

    # Remaining non-empty lines are edges
    for ln in lines[idx:]:
        ln = ln.strip()
        if not ln:
            continue
        parts = ln.split()
        if len(parts) < 5:
            continue
        try:
            x0, y0, x1, y1 = map(int, parts[:4])
            signal = parts[4]
        except Exception:
            continue
        edges.append(Edge(x0, y0, x1, y1, signal))

    if verbose:
        print(f"{GREEN}Parsed:{COLORRST} {W}x{H}, nodes={consumed_nodes}, edges={len(edges)}")
        if consumed_nodes != W*H:
            print(f"{YELLOW}Warning:{COLORRST} expected {W*H} nodes, parsed {consumed_nodes}.")

    return W, H, nodes, edges

def _resolve_color(sig: Optional[str], cycle: List[str]) -> str:
    if sig is None:
        return "0.7"
    # exact or case-insensitive map
    if sig in SIGNAL_COLORS and SIGNAL_COLORS[sig] != "none":
        return SIGNAL_COLORS[sig]
    for k, v in SIGNAL_COLORS.items():
        if k.lower() == sig.lower() and v != "none":
            return v
    # fallback stable cycle
    if not cycle:
        return "0.3"
    idx = abs(hash(sig)) % len(cycle)
    return cycle[idx]

def render_pdn_layer(
    path: str,
    out_path: Optional[str] = None,
    dpi: int = 200,
    show: bool = False,
    cell_size: float = 1.0,      # legacy radius scale
    linewidth: float = 1.6,
    edge_width: float = 1.6,
    no_legend: bool = True,
    no_title: bool = True,
    highlight_empty: bool = False,   # unused
    verbose: bool = False,
    highlight_edge_mode: bool = False,  # NEW: when True, emphasize edges
):
    from matplotlib.collections import LineCollection, PatchCollection
    from matplotlib.patches import Circle

    W, H, nodes, edges = parse_pdn_text(path, verbose=verbose)

    # Sizing from pxPerCell and lineScale
    try:
        px_per_cell = float(os.environ.get("PDN_PX_PER_CELL", "8"))
    except Exception:
        px_per_cell = 8.0
    try:
        line_scale = float(os.environ.get("PDN_LINE_SCALE", "1.0"))
    except Exception:
        line_scale = 1.0
    transparent = os.environ.get("PDN_TRANSPARENT", "0") == "1"

    fig_w_px = max(200, int(W * px_per_cell))
    fig_h_px = max(200, int(H * px_per_cell))
    fig_w_in = fig_w_px / dpi
    fig_h_in = fig_h_px / dpi

    fig = plt.figure(figsize=(fig_w_in, fig_h_in), facecolor="white")
    ax = plt.gca()

    cycle = plt.rcParams['axes.prop_cycle'].by_key().get('color', [])

    # Helpers
    def center(i, j):
        x = (i + 0.5) * cell_size
        y = (j + 0.5) * cell_size
        return x, y

    # --- Nodes: filled circles ---
    # In highlight-edge mode, shrink nodes so edges stand out.
    r_node = (0.18 if highlight_edge_mode else 0.35) * cell_size

    node_patches = []
    node_colors  = []
    for j in range(H):
        for i in range(W):
            node = nodes.get(NodeRef(i, j))
            if not node:
                continue
            cx, cy = center(i, j)
            col = _resolve_color(node.signal, cycle) if not _is_empty(node.signal) else "white"
            node_patches.append(Circle((cx, cy), r_node))
            node_colors.append(col)

    if node_patches:
        pc_nodes = PatchCollection(
            node_patches,
            facecolors=node_colors,
            edgecolors='none',
            alpha=0.95
        )
        ax.add_collection(pc_nodes)

    # --- Edges ---
    segs = []
    ecolors = []

    for e in edges:
        x0, y0 = center(e.x0, e.y0)
        x1, y1 = center(e.x1, e.y1)

        # shrink endpoints so they don't overlap node circles
        dx, dy = (x1 - x0), (y1 - y0)
        dist = sqrt(dx*dx + dy*dy)
        if dist <= 1e-6:
            continue

        shrink = r_node * 0.6  # slight spacing beyond node radius
        x0s = x0 + dx/dist * shrink
        y0s = y0 + dy/dist * shrink
        x1s = x1 - dx/dist * shrink
        y1s = y1 - dy/dist * shrink

        sig = e.signal
        if highlight_edge_mode and _is_empty(sig):
            # Skip empty edges entirely in highlight mode
            continue

        segs.append([(x0s, y0s), (x1s, y1s)])
        ecolors.append('0.6' if _is_empty(sig) else _resolve_color(sig, cycle))

    if segs:
        # In highlight mode, make edges thicker and slightly more opaque.
        base_width = (line_scale * 0.25) if highlight_edge_mode else (r_node * 0.3)
        lc = LineCollection(
            segs,
            colors=ecolors,
            linewidths=base_width,
            alpha=0.9 if highlight_edge_mode else 0.8
        )
        ax.add_collection(lc)

    # Axes tidy
    ax.set_aspect('equal', adjustable='box')
    ax.set_xlim(0, W*cell_size)
    ax.set_ylim(0, H*cell_size)
    ax.set_xticks([])
    ax.set_yticks([])
    if not no_title:
        ax.set_title("PDN Layer (nodes & links)")

    # Optional legend (kept off by default to avoid clutter)
    if not no_legend:
        uniq = sorted({nodes[n].signal for n in nodes if not _is_empty(nodes[n].signal)})
        handles = []
        labels = []
        for sig in uniq[:16]:
            col = _resolve_color(sig, cycle)
            line, = ax.plot([], [], linewidth=2.0, color=col, label=sig)
            handles.append(line); labels.append(sig)
        if handles:
            ax.legend(handles, labels, loc='upper right', frameon=False, fontsize=8)

    if out_path:
        if verbose:
            print(f"{YELLOW}Saving:{COLORRST} {out_path} @ {dpi} dpi")
        fig.savefig(out_path, dpi=dpi, bbox_inches="tight",
                    facecolor=('white' if not transparent else None),
                    transparent=transparent)
    if show and not out_path:
        plt.show()
    plt.close(fig)
    return out_path

def main(argv=None):
    parser = ArgumentParser(description="Visualise ObjectArray (fast). Draw node signal fills and colored links.")
    parser.add_argument("-i", "--input", required=True, help="Path to input file (required).")
    parser.add_argument("-o", "--output", required=False, help="Path to output image file (optional).")
    parser.add_argument("--dpi", type=int, default=200, required=False, help="DPI for output image (default: 200).")
    parser.add_argument("-p", "--pinSize", type=float,  default=1.0, required=False, help="Legacy: scales node radius (kept for compatibility).")
    parser.add_argument("--pxPerCell", type=float, default=10.0, required=False, help="Pixels per grid cell (default: 10).")
    parser.add_argument("--lineScale", type=float, default=1.2, required=False, help="Multiply edge widths (default: 1.2).")
    parser.add_argument("--transparent", action="store_true", help="Save figure with transparent background.")
    parser.add_argument("-v", "--verbose", action="store_true", help="Enable verbose mode.")
    parser.add_argument("--noLegend", action="store_true", help="Legend is omitted.")
    parser.add_argument("--noTitle", action="store_true", help="Title is omitted.")
    # NEW flag:
    parser.add_argument("-he", "--highlightEdge", action="store_true",
                        help="Emphasize edges: smaller nodes, thicker edges, and skip empty edges.")
    args = parser.parse_args(argv)

    # Pass sizing through env for simplicity
    os.environ["PDN_PX_PER_CELL"] = str(max(1.0, args.pxPerCell))
    os.environ["PDN_LINE_SCALE"] = str(max(0.1, args.lineScale))
    os.environ["PDN_TRANSPARENT"] = "1" if args.transparent else "0"

    render_pdn_layer(
        path=args.input,
        out_path=args.output,
        dpi=args.dpi,
        show=False,
        cell_size=args.pinSize,
        linewidth=1.6,
        edge_width=1.6,
        no_legend=args.noLegend,
        no_title=args.noTitle,
        highlight_empty=False,
        verbose=args.verbose,
        highlight_edge_mode=args.highlightEdge,  # << hook up new flag
    )
    if args.verbose:
        print(f"{GREEN}Done.{COLORRST}")

if __name__ == "__main__":
    main()