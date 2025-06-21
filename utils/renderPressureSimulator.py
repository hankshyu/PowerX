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
from dataclasses import dataclass
from typing import List
from typing import Optional
from dataclasses import dataclass, field

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

from dataclasses import dataclass
from typing import List

@dataclass
class Cord:
    x: float = 0.0
    y: float = 0.0

@dataclass
class SoftBody:
    id: int = -1
    sigType: str = ""
    expectCurrent: float = 0.0
    initialArea: float = 0.0
    pressure: float = 0.0
    contourCount: int = 0
    contour: List[Cord] = field(default_factory=list)

@dataclass
class ViaBody:
    position: Cord = Cord(-1, -1)
    sigType: str = ""
    upSoftBody: SoftBody = field(default_factory=SoftBody)
    downSoftBody: SoftBody = field(default_factory=SoftBody)
    viaStatus: str = ""

def parse_arguments():
    parser = argparse.ArgumentParser(description="Plot the Pressure Simulation System from PowerX")
    
    parser.add_argument("-i", "--input", required=True, help="Path to input file (required).")
    parser.add_argument("-o", "--output", help="Path to output image file (optional).")
    parser.add_argument("--dpi", type=int, default=400, help="DPI for output image (default: 400).")
    parser.add_argument("-v", "--verbose", action="store_true", help="Enable verbose mode.")
    parser.add_argument("--noLegend", action="store_true", help="Legend is omitted.")
    parser.add_argument("--noTitle", action="store_true", help="Title is omitted.")
    
    return parser.parse_args()


def parse_contour_line(line: str, contourCount: int) -> List[Cord]:
    # Use regular expression to extract all coordinate pairs
    matches = re.findall(r'\(\s*([-\d.]+)\s*,\s*([-\d.]+)\s*\)', line)
    
    # Report error if count doesn't match
    if len(matches) != contourCount:
        raise ValueError(f"Expected {contourCount} coordinates, but found {len(matches)}")

    # Convert to list of Cord
    contour = [Cord(float(x), float(y)) for x, y in matches]
    return contour


def append_path_from_cord_list(cordList: List[Cord]):
    vertices = []
    codes = []

    if not cordList:
        return vertices, codes

    vertices.append((cordList[0].x * GRID_MUL, cordList[0].y * GRID_MUL))
    codes.append(Path.MOVETO)

    for pt in cordList[1:]:
        vertices.append((pt.x * GRID_MUL, pt.y * GRID_MUL))
        codes.append(Path.LINETO)

    # Close the path
    vertices.append((cordList[0].x * GRID_MUL, cordList[0].y * GRID_MUL))
    codes.append(Path.CLOSEPOLY)

    return vertices, codes

if __name__ == "__main__":

    args = parse_arguments()
    fig, ax = plt.subplots(figsize=(30, 30))

    BORDER_WIDTH = 2
    GRID_MUL = 10
    POINT_RAIDUS = 4
    LINE_WIDTH = 1
    
    planeWidth = 0
    planeHeight = 0

    idToSoftBody : dict[int, SoftBody] = {}

    # start rendering progress
    try:
        with open(args.input, 'r') as filein:
            renderMode = ""
            # Read the first Line to determine the render mode
            LineBuffer = filein.readline().strip().split()
            if not (((LineBuffer[0] == "PRESSURE_SIMULATOR_SOFTBODY") or (LineBuffer[0] == "PRESSURE_SIMULATOR_SOFTBODY_WITH_PIN") or (LineBuffer[0] == "PRESSURE_SIMULATOR_SOFTBODY_WITH_PINS")) and (LineBuffer[1] == "VISUALISATION")):
                print("Error: Render mode unrecognized")
                exit()
            
            renderMode = LineBuffer[0] + " " + LineBuffer[1]
            planeWidth = int(LineBuffer[2])
            planeHeight = int(LineBuffer[3])

            # display the arguments if user specifies verbose mode
            if args.verbose:
                print(CYAN,"IRISLAB Pressure Simulator Rendering Program ", COLORRST)
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
            
            # Start reading metal layer related information
            LineBuffer = filein.readline().strip().split()
            if(not ((LineBuffer[0] == "METAL_LAYER") and (LineBuffer[1] == "SHAPES"))):
                print("[RenderPressureSimulator] Error: Missing Metal layer section")
                exit()
            
            shapes = int(LineBuffer[2])
            for i in range(shapes):
                LineBuffer = filein.readline().strip().split()
                
                readID = int(LineBuffer[0])
                readSigType = str(LineBuffer[1])
                readExpectCurrent = float(LineBuffer[2])
                readInitialArea = float(LineBuffer[3])

                newSB = SoftBody(readID, readSigType, readExpectCurrent, readInitialArea, 0, [])
                
                LineBuffer = filein.readline().strip().split()
                if(not (LineBuffer[0] == "pressure")):
                    print(f"[RenderPressureSimulator] Error: Missing pressure for metal layer with id = {readID}")
                    exit()
                newSB.pressure = float(LineBuffer[1])

                LineBuffer = filein.readline().strip().split()
                if(not ((LineBuffer[0] == "shape") and (int(LineBuffer[1]) > 0))):
                    print(f"[RenderPressureSimulator] Error: Missing shape for metal layer with id = {readID}")
                    exit()
                newSB.contourCount = int(LineBuffer[1])

                newSB.contour = parse_contour_line(filein.readline().strip(), newSB.contourCount)
                
                idToSoftBody[newSB.id] = newSB
            
            # plot the SoftBody on the canvas (render)
            ax.set_xlim(0, GRID_MUL*planeWidth)
            ax.set_ylim(0, GRID_MUL*planeHeight)
            ax.set_aspect('equal')
            ax.set_xticks([])
            ax.set_yticks([])

            for spine in ax.spines.values():
                spine.set_linewidth(BORDER_WIDTH)

            for sb in idToSoftBody.values():
                color = SIGNAL_COLORS.get(sb.sigType, "#CCCCCC")  # fallback to gray if unknown
                vertices, codes = append_path_from_cord_list(sb.contour)
                if not vertices or not codes:
                    continue

                path = Path(vertices, codes)
                patch = patches.PathPatch(path, facecolor=color, edgecolor='black', linewidth=LINE_WIDTH)
                ax.add_patch(patch)

            # Optional: Save or show
            if args.output:
                plt.savefig(args.output, dpi=args.dpi, bbox_inches='tight')
            else:
                plt.show()

    except FileNotFoundError:
        print(f"[RenderPressureSimulator]Error: File \"{args.input}\" or \"{args.output}\" not found")
        sys.exit()
    except PermissionError:
        print(f"[RenderPressureSimulator]Error: Permission denied when accessing \"{args.input}\"")
        sys.exit()