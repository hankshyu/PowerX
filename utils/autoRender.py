#!/usr/bin/env python3
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor, as_completed
import argparse
import os
import re
import shlex
import subprocess
import sys
from datetime import datetime
from typing import List, Dict, Tuple, Optional

INPUT_DIR = "./outputs/"

PATTERN_TO_LABEL: List[Tuple[str, str]] = [
    ("init_gawp_m*",       "01_Initial_m"),
    ("init_ps_m*",         "02_InitialPoints_m"),
    ("conn_ps_m*",         "03_ConnLayers_m"),
    ("route_ps_m*",        "04_Route_m"),
    ("reroute_ps_m*",      "05_ReRoute_m"),
    ("addvdpoints_ps_m*",  "06_AddVoronoiPoints_m"),
    ("genvd_vg_m*",        "07_GenerateVoronoiGraph_m"),
    ("mergevp_vp_m*",      "08_MergeVoronoiPolygons_m"),
    ("postp_gawp_m*",      "09_TranlateVoronoiResults_m"),
    ("leg_gawp_m*",        "10_Legalisation_m"),
    ("enhance_gawp_m*",    "11_ExchageOptimisation_m"),
    ("releg_gawp_m*",      "12_ReLegalisation_m"),
    ("postvd_gawp_m*",     "13_PostProcessing_m"),
    ("phyrlz_pi_m*",       "14_InitialPhysicalImplementation_m"),
    ("phyrlz_pi2_m*",      "15_PhysicalImplementation_m"),
    ("fnl_fnl_m*",         "16_FinalImplementation_m"),

    ("2init_gawp_m*",           "01_Initial_m"),
    ("2fillobst_gawp_m*",       "02_FillObstacles_m"),
    ("2fillEnclosed_gawp_m*",   "03_FillEnclosedRegions_m"),
    ("2mcfraw_gawp_m*",         "04_MCF_m"),
    ("2rfill_gawp_m*",          "05_ResistorNetworkBasedFilling_m"),
    ("2postp_gawp_m*",          "06_PostProcessing_m"),
    ("2phyrlz_pi_m*",           "07_InitialPhysicalImplementation_m"),
    ("2phyrlz_pi2_m*",          "08_PhysicalImplementation_m"),
    ("2fnl_fnl_m*",             "09_FinalImplementation_m")

]

_M_SUFFIX_RE = re.compile(r"_m(\d+)$")

def find_matches(base: Path, pattern: str, recursive: bool = False) -> List[Path]:
    globber = base.rglob if recursive else base.glob
    files = [p for p in globber(pattern) if p.is_file()]
    files.sort(key=lambda p: p.stat().st_mtime, reverse=True)
    return files

def classify_kind(p: Path) -> Optional[str]:
    name = p.name
    if "_gawp_" in name:
        return "gawp"
    if "_ps_" in name:
        return "ps"
    if "_vg_" in name:
        return "vg"
    if "_vp_" in name:
        return "vp"
    if "_pi_" in name:
        return "pi"
    if "_fnl_" in name:
        return "fnl"
    if name.startswith("final_m"):         # treat final_m* as gawp
        return "gawp"
    return None

def extract_m_suffix(stem: str) -> str:
    m = _M_SUFFIX_RE.search(stem)
    return f"m{m.group(1)}" if m else ""

def render_command(kind: str, infile: Path, outfile: Path, dpi: int, pin_size: float) -> List[str]:
    if kind == "ps":
        cmd = f"python3 utils/renderVoronoiPointsSegments.py -i {shlex.quote(str(infile))} -o {shlex.quote(str(outfile))} --dpi {dpi}"
    elif kind == "gawp":
        cmd = f"python3 utils/renderObjectArray.py -g -i {shlex.quote(str(infile))} -o {shlex.quote(str(outfile))} --pinSize {pin_size} --dpi {dpi}"
    elif kind == "vg":
        cmd = f"python3 utils/renderVoronoiGraph.py -i {shlex.quote(str(infile))} -o {shlex.quote(str(outfile))} --dpi {dpi}"
    elif kind == "vp":
        cmd = f"python3 utils/renderVoronoiPolygon.py -i {shlex.quote(str(infile))} -o {shlex.quote(str(outfile))} --dpi {dpi}"
    elif kind == "pi":
        cmd = f"python3 utils/renderPhysicalImplementation.py -i {shlex.quote(str(infile))} -o {shlex.quote(str(outfile))} --dpi {dpi} --pxPerCell 40 --lineScale 3 --noLegend --noTitle"
    elif kind == "fnl":
        cmd = f"python3 utils/renderFinalOutput.py -i {shlex.quote(str(infile))} -o {shlex.quote(str(outfile))} --dpi {dpi} --pxPerCell 40 --lineScale 3 --noLegend --noTitle -he"
    else:
        raise ValueError(f"Unknown kind: {kind}")
    return shlex.split(cmd)

def run_one(infile: Path, kind: str, outfile: Path, timeout: int, dpi: int, pin_size: float) -> Tuple[Path, str, Path, int, str, str]:
    try:
        cmd = render_command(kind, infile, outfile, dpi=dpi, pin_size=pin_size)
        proc = subprocess.run(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            timeout=timeout,
            check=False,
        )
        return (infile, kind, outfile, proc.returncode, proc.stdout, proc.stderr)
    except subprocess.TimeoutExpired as e:
        return (infile, kind, outfile, 124, e.stdout or "", f"TIMEOUT after {timeout}s")
    except FileNotFoundError as e:
        return (infile, kind, outfile, 127, "", f"COMMAND NOT FOUND: {e}")
    except Exception as e:
        return (infile, kind, outfile, 1, "", f"ERROR: {e}")

def dir_is_empty(p: Path) -> bool:
    try:
        next(p.iterdir())
        return False
    except StopIteration:
        return True

def main():
    parser = argparse.ArgumentParser(
        description="Render per-kind images from outputs/ into outputs/<timestamp> (quiet by default)."
    )
    parser.add_argument("-d", "--dir", default=INPUT_DIR, help="Input directory (default: outputs/)")
    parser.add_argument("-r", "--recursive", action="store_true", help="Recurse subdirectories")
    parser.add_argument("--show-size", action="store_true", help="Show sizes in listing (only if --show-scan)")

    # verbosity
    parser.add_argument("--show-scan", action="store_true", help="Show scan banner and grouped listings")
    parser.add_argument("--show-plan", action="store_true", help="Show planned commands before execution")
    parser.add_argument("--dry-run", action="store_true", help="List & plan only; do not execute")

    # parallelism & runtime
    parser.add_argument("--scan-workers", type=int, help="Threads for scanning (default: min(8, 2*CPU), cap 16)")
    parser.add_argument("--exec-workers", type=int, help="Threads for rendering; auto-detected if omitted (cap 8)")
    parser.add_argument("--timeout", type=int, default=1800, help="Per-file timeout seconds (default: 1800)")

    # output
    parser.add_argument("--out-root", default="outputs", help="Root output dir (default: outputs)")
    parser.add_argument("--stamp", default=None, help="Custom timestamp folder name (default: now)")
    parser.add_argument("--dpi", type=int, default=640, help="DPI for renders")
    parser.add_argument("--pin-size", type=float, default=3.5, help="Pin size for gawp renders")

    args = parser.parse_args()

    base = Path(args.dir)
    if not base.exists() or not base.is_dir():
        print(f"ERROR: Input directory not found or not a directory: {base}", file=sys.stderr)
        sys.exit(1)

    cpu = os.cpu_count() or 2
    scan_workers = args.scan_workers if args.scan_workers and args.scan_workers > 0 else min(8, 2 * cpu)
    scan_workers = min(scan_workers, 16)
    if args.exec_workers and args.exec_workers > 0:
        exec_workers = args.exec_workers
    else:
        exec_workers = min(8, max(1, cpu - 1))
    exec_workers = min(exec_workers, 8)

    stamp = args.stamp or datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
    out_dir = Path(args.out_root) / stamp
    try:
        out_dir.mkdir(parents=True, exist_ok=True)
    except Exception as e:
        print(f"ERROR: Could not create output directory '{out_dir}': {e}", file=sys.stderr)
        sys.exit(1)
    created_dir = out_dir

    # 1) Scan
    pattern_to_results: Dict[str, List[Path]] = {pat: [] for (pat, _) in PATTERN_TO_LABEL}
    with ThreadPoolExecutor(max_workers=scan_workers) as ex:
        futures = {ex.submit(find_matches, base, pat, args.recursive): pat for (pat, _) in PATTERN_TO_LABEL}
        for fut in as_completed(futures):
            pat = futures[fut]
            try:
                pattern_to_results[pat] = fut.result()
            except Exception:
                pattern_to_results[pat] = []

    # Optional: print grouped listings
    if args.show_scan:
        from datetime import datetime as _dt
        def _fmt(ts: float) -> str:
            return _dt.fromtimestamp(ts).strftime("%Y-%m-%d %H:%M:%S")
        print(f"Scanning: {base.resolve()} | recursive={args.recursive} | scan_workers={scan_workers}\n")
        for pattern, label in PATTERN_TO_LABEL:
            files = pattern_to_results.get(pattern, [])
            print(f"=== {label} (pattern: '{pattern}') — {len(files)} file(s) ===")
            if not files:
                print("  (no matches)\n")
                continue
            for p in files:
                try:
                    st = p.stat()
                    t = _fmt(st.st_mtime)
                    if args.show_size:
                        print(f"  {p}    [{t}]    {st.st_size} B")
                    else:
                        print(f"  {p}    [{t}]")
                except FileNotFoundError:
                    print(f"  {p}    [missing]")
            print()

    # If nothing found at all, cleanup and exit
    any_found = any(pattern_to_results[pat] for (pat, _) in PATTERN_TO_LABEL)
    if not any_found:
        if created_dir.exists() and dir_is_empty(created_dir):
            try:
                created_dir.rmdir()
            except Exception:
                pass
        print("No matching files found. Nothing to render.")
        return

    # 2) Build tasks (input, kind, output path)
    tasks: List[Tuple[Path, str, Path]] = []
    for pattern, label in PATTERN_TO_LABEL:
        for infile in pattern_to_results.get(pattern, []):
            kind = classify_kind(infile)
            if not kind:
                continue
            stem = infile.stem
            m_suffix = extract_m_suffix(stem)
            out_name = f"{label}{m_suffix}.png" if m_suffix else f"{label}.png"
            tasks.append((infile, kind, out_dir / out_name))

    # Optional: plan preview (only if asked or dry-run)
    if args.show_plan or args.dry_run:
        print(f"Output folder: {out_dir.resolve()}\n")
        for infile, kind, outfile in tasks:
            try:
                cmd_preview = " ".join(render_command(kind, infile, outfile, args.dpi, args.pin_size))
            except Exception as e:
                cmd_preview = f"(error building command: {e})"
            print(f"[PLAN] {kind.upper():4s}  IN: {infile}  →  OUT: {outfile.name}")
            print(f"       CMD: {cmd_preview}")
        print()

    if args.dry_run:
        # cleanup empty dir if nothing was executed
        if created_dir.exists() and dir_is_empty(created_dir):
            try:
                created_dir.rmdir()
            except Exception:
                pass
        return

    # 3) Execute with progress (x/N)
    total = len(tasks)
    print(f"Rendering {total} file(s) with exec_workers={exec_workers}, timeout={args.timeout}s...")
    done = 0
    results = []
    with ThreadPoolExecutor(max_workers=exec_workers) as ex:
        fut_to_item = {
            ex.submit(run_one, infile, kind, outfile, args.timeout, args.dpi, args.pin_size): (infile, kind, outfile)
            for (infile, kind, outfile) in tasks
        }
        for fut in as_completed(fut_to_item):
            infile, kind, outfile = fut_to_item[fut]
            try:
                _infile, _kind, _outfile, code, out, err = fut.result()
                done += 1
                status = "OK" if code == 0 else f"FAIL({code})"
                print(f"({done}/{total}) [{status}] {kind.upper():4s} :: {infile.name} → {outfile.name}")
                if code != 0 and err:
                    print(f"  stderr: {err.strip()}")
                results.append((_infile, _kind, _outfile, code, out, err))
            except Exception as e:
                done += 1
                print(f"({done}/{total}) [FAIL(?)] {kind.upper():4s} :: {infile.name} → {outfile.name} :: ERROR {e}")
                results.append((infile, kind, outfile, 1, "", f"EXCEPTION: {e}"))

    ok = sum(1 for *_rest, code, _o, _e in results if code == 0)
    print(f"Done. Success: {ok}/{total} | Failed: {total - ok}/{total}")
    print(f"Rendered images are in: {out_dir.resolve()}")

    # 4) Remove the timestamp dir if empty
    try:
        if created_dir.exists() and dir_is_empty(created_dir):
            created_dir.rmdir()
            print(f"(Output directory was empty and has been removed: {created_dir})")
    except Exception:
        pass

if __name__ == "__main__":
    main()