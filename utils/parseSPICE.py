#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Run an experiment end-to-end:
1) Upload the .sp files for a chosen case (case01..case06) to a remote host
2) Run HSPICE remotely and fetch the resulting .lis files to ./exp
3) Parse the results, print the report, and render the chip visualization

Usage:
  python runExperiment.py case01 [-o out.png] [--noShow]
  python runExperiment.py case02 --no-remote        # only parse existing exp/*.lis

Notes:
- Local source *.sp files are expected in ./outputs/
- Remote working directory is PowerS/
- Downloaded *.lis files are stored in ./exp/
- IR-drop values are reported in mV (4 decimals), power in W (4 decimals)
"""

import sys
import re
import subprocess
from pathlib import Path
from typing import List, Tuple, Dict, Optional

# Remote execution deps
import paramiko
from scp import SCPClient

# ----------------------------
# Config
# ----------------------------
# Remote connection (edit as needed)
hostname = '140.112.20.243'
port = 10073
# hostname = '192.168.48.73'
# port = 22

username = 'orange'
password = 'irislab123'

# Paths
BASE_DIR = 'exp/'         # local dir for .lis results
LOCAL_SP_DIR = Path('outputs')
REMOTE_DIR = 'PowerS/'    # remote working dir

# Case presets
case_to_spfiles = {
    'case01': ["POWER_1.sp", "POWER_2.sp", "POWER_3.sp"],
    'case02': ["POWER_1.sp", "POWER_2.sp"],
    'case03': ["POWER_1.sp", "POWER_2.sp", "POWER_3.sp"],
    'case04': ["POWER_1.sp", "POWER_2.sp", "POWER_3.sp", "POWER_4.sp"],  # duplicate per spec
    'case05': ["POWER_1.sp", "POWER_2.sp", "POWER_3.sp"],
    'case06': ["POWER_1.sp", "POWER_2.sp", "POWER_3.sp"],
}

# For parsing/visualization we map cases to expected local .lis paths
experiment = {
    'case01': [BASE_DIR+"POWER_1.lis", BASE_DIR+"POWER_2.lis", BASE_DIR+"POWER_3.lis"],
    'case02': [BASE_DIR+"POWER_1.lis", BASE_DIR+"POWER_2.lis"],
    'case03': [BASE_DIR+"POWER_1.lis", BASE_DIR+"POWER_2.lis", BASE_DIR+"POWER_3.lis"],
    'case04': [BASE_DIR+"POWER_1.lis", BASE_DIR+"POWER_2.lis", BASE_DIR+"POWER_3.lis", BASE_DIR+"POWER_4.lis"],
    'case05': [BASE_DIR+"POWER_1.lis", BASE_DIR+"POWER_2.lis", BASE_DIR+"POWER_3.lis"],
    'case06': [BASE_DIR+"POWER_1.lis", BASE_DIR+"POWER_2.lis", BASE_DIR+"POWER_3.lis"],
}

canvasSize = {
    # width, height
    'case01': [126, 126],
    'case02': [126, 126],
    'case03': [126, 126],
    'case04': [122, 122],
    'case05': [128, 128],
    'case06': [179, 179],
}
SIGNAL_COLORS = {
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
}

# ----------------------------
# CLI
# ----------------------------
def parse_cli(argv: List[str]) -> Tuple[str, Optional[Path], bool, bool]:
    """Return (case_key, output_path, no_show, do_remote)."""
    if len(argv) < 2:
        print("Usage:")
        print("  python runExperiment.py case01 [-o out.png] [--noShow]")
        print("  python runExperiment.py case01 --no-remote  # skip remote run, just parse/plot")
        sys.exit(1)
    case_key: Optional[str] = None
    out: Optional[Path] = None
    no_show = False
    do_remote = True
    i = 1
    while i < len(argv):
        tok = argv[i]
        if tok in ("-o", "--output"):
            if i+1 >= len(argv):
                print("[error] -o/--output requires a path")
                sys.exit(2)
            out = Path(argv[i+1])
            i += 2
        elif tok == "--noShow":
            no_show = True
            i += 1
        elif tok == "--no-remote":
            do_remote = False
            i += 1
        else:
            if case_key is None:
                case_key = tok
            else:
                print(f"[error] Unexpected argument: {tok}")
                sys.exit(2)
            i += 1
    if case_key is None:
        print("[error] Missing case key (case01..case06)")
        sys.exit(1)
    if case_key not in experiment or case_key not in case_to_spfiles:
        print(f"[error] Unknown case '{case_key}'. Choose one of: {', '.join(sorted(experiment.keys()))}")
        sys.exit(1)
    return case_key, out, no_show, do_remote

# ----------------------------
# Remote helpers
# ----------------------------
def ensure_dirs():
    Path(BASE_DIR).mkdir(parents=True, exist_ok=True)
    LOCAL_SP_DIR.mkdir(parents=True, exist_ok=True)

def make_ssh_client() -> paramiko.SSHClient:
    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    ssh.connect(hostname, port, username, password)
    return ssh

def build_remote_commands(sp_names: List[str]) -> str:
    # Source env and run hspice for each file
    cmds = ["source /nfs/home/cad/synopsys/CIC/hspice.cshrc"]
    for sp in sp_names:
        lis = sp.replace('.sp', '.lis')
        cmds.append(f"hspice -i {REMOTE_DIR}{sp} -o {REMOTE_DIR}{lis}")
    return " ; ".join(cmds)

def run_remote(case_key: str):
    sp_list = case_to_spfiles[case_key]
    # Upload .sp files
    ssh = make_ssh_client()
    try:
        with SCPClient(ssh.get_transport()) as scp:
            for sp in sp_list:
                src = LOCAL_SP_DIR / sp
                if not src.exists():
                    raise FileNotFoundError(f"Missing local SPICE file: {src}")
                scp.put(str(src), REMOTE_DIR)
    finally:
        ssh.close()

    # Execute commands
    client = paramiko.SSHClient()
    client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    try:
        client.connect(hostname, port, username, password)
        full_cmd = build_remote_commands(sp_list)
        stdin, stdout, stderr = client.exec_command(f'tcsh -c "{full_cmd}"')
        print("STDOUT:"); print(stdout.read().decode(errors="ignore"))
        print("STDERR:"); print(stderr.read().decode(errors="ignore"))
    finally:
        client.close()

    # Download .lis files
    ssh = make_ssh_client()
    try:
        with SCPClient(ssh.get_transport()) as scp:
            for sp in sp_list:
                lis = sp.replace('.sp', '.lis')
                scp.get(REMOTE_DIR + lis, str(Path(BASE_DIR) / lis))
    finally:
        ssh.close()

# ----------------------------
# Parsing + Visualization
# ----------------------------
def tag_from_path(p: Path) -> str:
    return p.stem  # e.g., POWER_2

def extract_irdrop_lines_filtered(text: str) -> List[str]:
    """Return lines that start with 'irdrop', excluding 'irdroppcb='."""
    out = []
    for ln in text.splitlines():
        s = ln.strip()
        sl = s.lower()
        if sl.startswith("irdrop") and not sl.startswith("irdroppcb"):
            out.append(s)
    return out

VALUE_FLOAT = r'[-+]?(?:\d+\.?\d*|\.\d+)(?:[Ee][-\+]?\d+)?'
_value_pat = re.compile(r'=\s*(' + VALUE_FLOAT + r')')

def parse_irdrop_records(lines: List[str], tag: str) -> List[Dict[str, float]]:
    """
    From IR-drop lines like:
      irdrop01gpu1_4_4_42_42_2.95= 1.5060e-01
    return dicts with chiplet, x, y, w, h, current, drop_v (V), tag, line.
    """
    out: List[Dict[str, float]] = []
    for s in lines:
        left_right = s.split('=', 1)
        left = left_right[0].strip()
        if not left.lower().startswith('irdrop'):
            continue
        left_body = left[len('irdrop'):]
        parts = left_body.split('_')
        if len(parts) < 6:
            continue
        chiplet = parts[0]
        try:
            x = float(parts[1]); y = float(parts[2])
            w = float(parts[3]); h = float(parts[4])
            current = float(parts[5])
        except ValueError:
            continue
        mval = _value_pat.search(s)
        if not mval:
            continue
        try:
            drop_v = float(mval.group(1))
        except ValueError:
            continue
        out.append({
            "chiplet": chiplet,
            "x": x, "y": y, "w": w, "h": h,
            "current": current,
            "drop_v": drop_v,
            "tag": tag,
            "line": s,
        })
    return out

def parse_resistor_powers(text: str) -> Tuple[float, Dict[str, float]]:
    total = 0.0
    per_elem: Dict[str, float] = {}
    lines = text.splitlines()
    i = 0
    in_res = False
    current_elements: List[str] = []
    while i < len(lines):
        raw = lines[i]; line = raw.strip()
        if line.lower().startswith("**** resistors"):
            in_res = True; current_elements = []; i += 1; continue
        if in_res and line.startswith("****") and (not line.lower().startswith("**** resistors")):
            in_res = False; current_elements = []; i += 1; continue
        if not in_res: i += 1; continue
        low = line.lower()
        if low.startswith("element"):
            after = raw.split("element", 1)[1].strip()
            toks = [t for t in re.split(r'\s+', after) if t]
            current_elements = toks
        elif low.startswith("power"):
            after = raw.split("power", 1)[1].strip()
            vals = [t for t in re.split(r'[\s,]+', after) if t]
            nums = []
            for t in vals:
                try: nums.append(float(t))
                except ValueError: pass
            if current_elements and nums:
                n = min(len(current_elements), len(nums))
                for k in range(n):
                    name = current_elements[k]; val = nums[k]
                    per_elem[name] = per_elem.get(name, 0.0) + val
                    total += val
        i += 1
    return total, per_elem

def parse_and_visualize(case_key: str, out_path: Optional[Path], no_show: bool):
    files = [Path(x) for x in experiment[case_key]]
    # Accumulators
    ir_lines_tagged: List[str] = []
    perfile_totals: List[Tuple[str, float]] = []
    grand_total = 0.0
    global_elem_records: List[Tuple[str, str, float]] = []
    all_records: List[Dict[str, float]] = []

    for fpath in files:
        tag = tag_from_path(fpath)
        if not fpath.exists():
            perfile_totals.append((tag, float('nan')))
            continue
        txt = fpath.read_text(errors="ignore")
        filtered = extract_irdrop_lines_filtered(txt)
        for ln in filtered:
            ir_lines_tagged.append(f"[{tag}] {ln}")
        all_records.extend(parse_irdrop_records(filtered, tag))
        tot, per_elem = parse_resistor_powers(txt)
        perfile_totals.append((tag, tot))
        grand_total += tot
        for name, val in per_elem.items():
            global_elem_records.append((tag, name, val))

    # ---- Console report ----
    print(f"[info] Experiment mode: {case_key}")
    print("\n== IR-drop outputs (filtered) ==")
    if ir_lines_tagged:
        for s in ir_lines_tagged: print(s)
    else:
        print("(none found)")
    print("\n== Resistor power summary ==")
    for tag, tot in perfile_totals:
        if tot == tot: print(f"[{tag}] {tot} W")
        else: print(f"[{tag}] (file not found)")
    if len(files) > 1: print(f"Grand Total resistor power: {grand_total} W")
    if global_elem_records:
        print("\n=== Global Top-5 contributors (by |power|) ===")
        top5 = sorted(global_elem_records, key=lambda r: abs(r[2]), reverse=True)[:5]
        for tag, name, val in top5:
            print(f"[{tag}] {name:>16s}  |  type: resistor  |  {val} W")

    # Final report
    worst_val = max((r["drop_v"] for r in all_records), default=float('nan'))
    wsum = sum(r["current"] * r["drop_v"] for r in all_records)
    wtot = sum(r["current"] for r in all_records)
    wavg = (wsum / wtot) if wtot > 0 else float('nan')
    print("\nFinal report:")
    print(f"Worse IRDrop: {worst_val*1000:.4f} mV" if worst_val==worst_val else "Worse IRDrop: NaN")
    print(f"Weighted-Average IRDrop: {wavg*1000:.4f} mV" if wavg==wavg else "Weighted-Average IRDrop: NaN")
    print(f"Total Power Loss: {grand_total:.4f} W")

    # ---- Visualization ----
    if all_records:
        try:
            import matplotlib
            if out_path is not None or no_show: matplotlib.use("Agg")
            import matplotlib.pyplot as plt
            from matplotlib.patches import Rectangle
            # Canvas
            if case_key in canvasSize: W, H = canvasSize[case_key]
            else: W, H = 128, 128
            fig, ax = plt.subplots(figsize=(8, 8))
            ax.add_patch(Rectangle((0, 0), W, H, fill=False, linewidth=2, edgecolor="black"))
            # Draw chiplets
            for r in all_records:
                color = SIGNAL_COLORS.get(str(r["tag"]), "#808080")
                ax.add_patch(Rectangle((r["x"], r["y"]), r["w"], r["h"], facecolor=color, alpha=0.6, edgecolor="black", linewidth=0.5))
                cx = r["x"] + r["w"]/2.0; cy = r["y"] + r["h"]/2.0
                label = f'{r["current"]:.2f} A\n{(r["drop_v"]*1000):.2f} mV'
                ax.text(cx, cy, label, ha="center", va="center", fontsize=8, color="black")
            ax.set_xlim(0, W); ax.set_ylim(0, H); ax.set_aspect("equal", adjustable="box")
            ax.set_xticks([]); ax.set_yticks([]); ax.tick_params(labelbottom=False, labelleft=False)
            worse_mv = (worst_val*1000.0 if worst_val==worst_val else float("nan"))
            avg_mv = (wavg*1000.0 if wavg==wavg else float("nan"))
            title = f"{case_key.capitalize()}: Worse/Avg. IR drop = {worse_mv:.4f}/{avg_mv:.4f} mV  Total Power Loss = {grand_total:.4f} W"
            ax.set_title(title)
            fig.tight_layout()
            if out_path is not None:
                fig.savefig(out_path, dpi=200)
                print(f"[info] Saved visualization to: {out_path}")
            if not no_show and out_path is None:
                plt.show()
            plt.close(fig)
        except Exception as e:
            print(f"[warn] Visualization failed: {e}")

# ----------------------------
# Entry
# ----------------------------
def main():
    case_key, out_path, no_show, do_remote = parse_cli(sys.argv)
    ensure_dirs()
    if do_remote:
        print(f"[info] Running remote job for {case_key} ...")
        run_remote(case_key)
    else:
        print(f"[info] Skipping remote run; parsing existing results for {case_key}.")
    parse_and_visualize(case_key, out_path, no_show)

if __name__ == '__main__':
    main()
