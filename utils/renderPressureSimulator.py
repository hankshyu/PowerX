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
    x: float
    y: float

@dataclass
class SoftBody:
    id: int
    sigType: str
    expectCurrent: float
    initialArea: float
    pressure: float
    contour: List[Cord]

@dataclass
class ViaBody:
    position: Cord
    sigType: str
    upSoftBody: SoftBody
    downSoftBody: SoftBody
    viaStatus: str

def parse_arguments():
    parser = argparse.ArgumentParser(description="Plot the Pressure Simulation System from PowerX")
    
    parser.add_argument("-i", "--input", required=True, help="Path to input file (required).")
    parser.add_argument("-o", "--output", help="Path to output image file (optional).")
    parser.add_argument("--dpi", type=int, default=400, help="DPI for output image (default: 400).")
    parser.add_argument("-v", "--verbose", action="store_true", help="Enable verbose mode.")
    parser.add_argument("--noLegend", action="store_true", help="Legend is omitted.")
    parser.add_argument("--noTitle", action="store_true", help="Title is omitted.")
    
    return parser.parse_args()

if __name__ == "__main__":

    args = parse_arguments()
    fig, ax = plt.subplots(figsize=(30, 30))

    BORDER_WIDTH = 2
    GRID_MUL = 10
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
            if not (((LineBuffer[0] == "PRESSURE_SIMULATOR_SOFTBODY") or (LineBuffer[0] == "PRESSURE_SIMULATOR_SOFTBODY_WITH_PIN") or (LineBuffer[0] == "PRESSURE_SIMULATOR_SOFTBODY_WITH_PINS")) and (LineBuffer[1] == "VISUALISATION")):
                print("Error: Render mode unrecognized")
                exit()
            
            renderMode = LineBuffer[0] + " " + LineBuffer[1]
            planeWidth = int(LineBuffer[2])
            planeHeight = int(LineBuffer[3])

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
            

    except FileNotFoundError:
        print(f"[RenderPressureSimulator]Error: File \"{args.input}\" not found")
        sys.exit()
    except PermissionError:
        print(f"[RenderPinMap]Error: Permission denied when accessing \"{args.input}\"")
        sys.exit()