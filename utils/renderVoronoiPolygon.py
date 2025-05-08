import sys
sys.path.append("./lib/")
import argparse
import re
import queue
import random
import ast
import matplotlib.pyplot as plt
from matplotlib.path import Path
import matplotlib.patches as patches

COLORRST    = "\u001b[0m"
BLACK       = "\u001b[30m"
RED         = "\u001b[31m"
GREEN       = "\u001b[32m"
YELLOW      = "\u001b[33m"
BLUE        = "\u001b[34m"
MAGENTA     = "\u001b[35m"
CYAN        = "\u001b[36m"
WHITE       = "\u001b[37m"

# Define a global mapping for pin colors
SIGNAL_COLORS = {
    "CHIPLET": "#B8B8B8",
    "EMPTY": "none",
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

def parse_arguments():
    parser = argparse.ArgumentParser(description="Plot the Voronoi Polygon from PowerX")
    
    parser.add_argument("-i", "--input", required=True, help="Path to input file (required).")
    parser.add_argument("-o", "--output", help="Path to output image file (optional).")
    parser.add_argument("--dpi", type=int, default=400, help="DPI for output image (default: 400).")
    parser.add_argument("-v", "--verbose", action="store_true", help="Enable verbose mode.")
    parser.add_argument("--noLegend", action="store_true", help="Legend is omitted.")
    parser.add_argument("--noTitle", action="store_true", help="Title is omitted.")
    
    return parser.parse_args()

def signed_area(x, y):
    return 0.5 * sum(x[i] * y[(i + 1) % len(x)] - x[(i + 1) % len(x)] * y[i] for i in range(len(x)))

def correct_winding(x, y, desired_ccw=True):
    area = signed_area(x, y)
    if (area > 0) != desired_ccw:  # mismatch
        return x[::-1], y[::-1]
    return x, y

def append_path(x, y):
    points = list(zip(x, y))
    verts = [(pt[0], pt[1]) for pt in points]
    codes = [Path.MOVETO] + [Path.LINETO] * (len(points) - 2) + [Path.CLOSEPOLY]
    return verts, codes

if __name__ == "__main__":

    args = parse_arguments()
    fig, ax = plt.subplots(figsize=(30, 30))

    BORDER_WIDTH = 2
    GRID_MUL = 20
    POINT_RAIDUS = 4
    LINE_WIDTH = 1
    planeWidth = 0
    planeHeight = 0

    # start rendering progress
    try:
        with open(args.input, 'r') as filein:
            renderMode = ""
            # Read the first Line to determine the render mode
            LineBuffer = filein.readline().strip().split()
            if not ((LineBuffer[0] == "M5") or (LineBuffer[0] == "M7")):
                print("Error: Render mode unrecognized" + LineBuffer[0])
                exit()

            if not ((LineBuffer[1] == "VORONOI_POLYGON") and (LineBuffer[2] == "VISUALISATION")):
                print("Error: Render mode unrecognized" + LineBuffer[1] + " " + LineBuffer[2])
                exit()
                
            renderMode = LineBuffer[0] + " " + LineBuffer[1]
            planeWidth = int(LineBuffer[3]) - 1
            planeHeight = int(LineBuffer[4]) - 1

            # display the arguments if user specifies verbose mode
            if args.verbose:
                print(CYAN,"IRISLAB Power Plane Rendering Program ", COLORRST)
                print("Input File: ", GREEN, args.input, COLORRST)
                
                if args.output is None:
                    print("Output File: ", RED, "Not saved", COLORRST)
                else:
                    print("Output File: ", GREEN, args.output, COLORRST)
                
                if args.dpi is None:
                    print("Output Image dpi: ", GREEN, "400", COLORRST)
                else:
                    FIG_DPI = int(args.dpi)
                    print("Output Image dpi: ", GREEN, FIG_DPI, COLORRST)
                
                if args.noLegend:
                    print("Legend display: ", RED, "off", COLORRST)
                else:
                    print("Legend: ", GREEN, "on", COLORRST)
                    
                if args.noTitle:
                    print("Title display: ", RED, "off", COLORRST)
                else:
                    print("Title display: ", GREEN, "on", COLORRST)

                print(f"Render Mode: " + renderMode)
                print(f"Canvas Size: {planeWidth} x {planeHeight}")


            ax.set_xlim(0, GRID_MUL*planeWidth)
            ax.set_ylim(0, GRID_MUL*planeHeight)
            ax.set_aspect('equal')
            ax.set_xticks([])
            ax.set_yticks([])
            
            # Make plot border thicker
            for spine in ax.spines.values():
                spine.set_linewidth(BORDER_WIDTH)  # You can increase this value

            # ax.spines['top'].set_color('white')
            # ax.spines['right'].set_color('white')
            # ax.spines['bottom'].set_color('white')
            # ax.spines['left'].set_color('white')

            LineBuffer = filein.readline().strip().split()
            if LineBuffer[0] != "SIGNALS":
                print("Error: no SIGNAL label found")
                exit()
            
            signalCount = int(LineBuffer[1])
            
            for sig in range(signalCount):
                LineBuffer = filein.readline().strip().split()
                if LineBuffer[2] != "PIECES":
                    print("Error: Missing PIECES label")
                    exit()

                pieces = int(LineBuffer[1])
                color = SIGNAL_COLORS[LineBuffer[0]]

                for piece in range(pieces):
                    LineBuffer = filein.readline().strip().split()
                    if (LineBuffer[0] != "PIECE") or (int(LineBuffer[1]) >= pieces) or(LineBuffer[2] != "POINTS"):
                        print("Error: Parsing PIECE N POINTS M failed: ")
                        print(LineBuffer)
                        exit()
                    pointCount = int(LineBuffer[3])
                    pieceWindingX = []
                    pieceWindingY = []
                    for pointidx in range(pointCount):
                        LineBuffer = filein.readline().strip()
                        match = re.search(r"f\(([^,]+), ([^)]+)\)", LineBuffer)

                        if match:
                            x = float(match.group(1))
                            y = float(match.group(2))
                            pieceWindingX.append(GRID_MUL * x)
                            pieceWindingY.append(GRID_MUL * y)
                        else:
                            print("Signle point no match " + LineBuffer)
                            exit()

                    LineBuffer = filein.readline().strip().split()
                    if (LineBuffer[0] != "PIECE") or (int(LineBuffer[1]) >= pieces) or (LineBuffer[2] != "HOLES"):
                        print("Error: Parsing PIECE N HOLES M failed: ")
                        print(LineBuffer)
                        exit()
                    holesCount = int(LineBuffer[3])
                    holeWindingArrX = []
                    holeWindingArrY = []
                    for holeidx in range(holesCount):
                        LineBuffer = filein.readline().strip().split()
                        if(LineBuffer[0] != "HOLE") or (int(LineBuffer[2] != "POINTS")):
                            print("Error parsing HOLE M POINTS N")
                            print(LineBuffer)
                            exit()
                        holePointsCount = int(LineBuffer[3])
                        holewindingx = []
                        holewindingy = []
                        for holewindingcordidx in range(holePointsCount):
                            LineBuffer = filein.readline().strip()
                            match = re.search(r"f\(([^,]+), ([^)]+)\)", LineBuffer)

                            if match:
                                x = float(match.group(1))
                                y = float(match.group(2))
                                holewindingx.append(GRID_MUL * x)
                                holewindingy.append(GRID_MUL * y)
                            else:
                                print("Signle point no match " + LineBuffer)
                                exit()
                        holeWindingArrX.append(holewindingx)
                        holeWindingArrY.append(holewindingy)

                    pieceWindingX, pieceWindingY = correct_winding(pieceWindingX, pieceWindingY, desired_ccw=True)
                    vertices = []
                    codes = []

                    ext_verts, ext_codes = append_path(pieceWindingX, pieceWindingY)
                    vertices.extend(ext_verts)
                    codes.extend(ext_codes)

                    # Fix winding for holes to be CW
                    for hx, hy in zip(holeWindingArrX, holeWindingArrY):
                        hx, hy = correct_winding(hx, hy, desired_ccw=False)
                        hole_verts, hole_codes = append_path(hx, hy)
                        vertices.extend(hole_verts)
                        codes.extend(hole_codes)
                    path = Path(vertices, codes)
                    patch = patches.PathPatch(path=path, facecolor=color, edgecolor='black', linewidth=LINE_WIDTH)
                    ax.add_patch(patch)

            
            
            if args.output is not None:
                plt.savefig(args.output, dpi=args.dpi)
                plt.close()
                
            else:
                plt.show()

                

    except FileNotFoundError:
        print(f"[RenderPinMap]Error: File \"{args.input}\" not found")
        sys.exit()
    except PermissionError:
        print(f"[RenderPinMap]Error: Permission denied when accessing \"{args.input}\"")
        sys.exit()