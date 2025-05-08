import sys
sys.path.append("./lib/")
import argparse
import re
import queue
import random
import ast
import matplotlib.pyplot as plt
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
    parser = argparse.ArgumentParser(description="Plot the Voronoi Diagram from PowerX")
    
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
                print("Error: Render mode unrecognized")
                exit()

            if not ((LineBuffer[1] == "VORONOI_GRAPH") and (LineBuffer[2] == "VISUALISATION")):
                print("Error: Render mode unrecognized")
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
                sigName = filein.readline().strip()
                color = SIGNAL_COLORS[sigName]

                LineBuffer = filein.readline().strip().split()
                if not ((LineBuffer[0] == "POINTS")):
                    print("Error: POINTS label not found")
                    exit()
                pointCount = int(LineBuffer[1])
                for pc in range(pointCount):
                    cellCentre = filein.readline().strip()
                    cellCentreX = 0
                    cellCentreY = 0
                    match = re.match(r"\(?\s*(-?\d+)\s*,\s*(-?\d+)\s*\)?", cellCentre)
                    if match:
                        cellCentreX, cellCentreY = map(int, match.groups())
                        circle = plt.Circle((GRID_MUL*cellCentreX, GRID_MUL*cellCentreY), radius=POINT_RAIDUS, color=color, fill=True, alpha = 1.0)
                        ax.add_patch(circle)
                    else:
                        print("Centre cell no match " + cellCentre)
                        exit()
                    # process the surroundings
                    LineBuffer = filein.readline().strip()
                    if LineBuffer != "0":
                        matches = re.findall(r"f\(\s*(-?\d+(?:\.\d+)?)\s*,\s*(-?\d+(?:\.\d+)?)\s*\)", LineBuffer)
                        # Split into x and y vectors
                        x_vec = [float(x) for x, _ in matches]
                        y_vec = [float(y) for _, y in matches]
                        x_vec = [element * GRID_MUL for element in x_vec]
                        y_vec = [element * GRID_MUL for element in y_vec]
                        if (x_vec[0] != x_vec[-1]) or (y_vec[0] != y_vec[-1]):
                            x_vec.append(x_vec[0])
                            y_vec.append(y_vec[0])

                        plt.plot(x_vec, y_vec, color='black', linewidth = LINE_WIDTH)
                        plt.fill(x_vec, y_vec, facecolor = color, edgecolor='black', alpha = 0.6)
            
            
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