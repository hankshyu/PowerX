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
    "EMPTY": "none",
    "POWER_1": "#1e81b0",
    "POWER_2": "#e67e22",
    "POWER_3": "#ffc107",
    "POWER_4": "#b29dd9",
    "POWER_5": "#72f2ee",
    "POWER_6": "#fc83bc",
    "POWER_7": "#c0392b",
    "POWER_8": "#21b2ab",
    "POWER_9": "#b0f294",
    "POWER_10": "#8d57a3",
    "GROUND": "#5cb85c",
    "OBSTACLE": "#A0A0A0",
    "OVERLAP": "#ff0000",
    "UNKNOWN": "#000000",
}

def parse_arguments():
    parser = argparse.ArgumentParser(description="Plot the Voronoi Class's Points and Segments from PowerX")
    
    parser.add_argument("-i", "--input", required=True, help="Path to input file (required).")
    parser.add_argument("-o", "--output", help="Path to output image file (optional).")
    parser.add_argument("--dpi", type=int, default=400, help="DPI for output image (default: 400).")
    parser.add_argument("-v", "--verbose", action="store_true", help="Enable verbose mode.")
    parser.add_argument("--noLegend", action="store_true", help="Legend is omitted.")
    parser.add_argument("--noTitle", action="store_true", help="Title is omitted.")
    
    return parser.parse_args()



if __name__ == "__main__":

    args = parse_arguments()

        
    fig, ax = plt.subplots(figsize=(24, 24))

    BORDER_WIDTH = 2
    GRID_MUL = 10
    POINT_RAIDUS = 3
    LINE_WIDTH = 1.5
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

            if not ((LineBuffer[1] == "VORONOI") and (LineBuffer[2] == "VISUALISATION")):
                print("Error: Render mode unrecognized")
                exit()
                
            renderMode = LineBuffer[0] + " " + LineBuffer[1]
            planeWidth = int(LineBuffer[3])
            planeHeight = int(LineBuffer[4])

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

                # render the Segment part
                LineBuffer = filein.readline().strip().split()
                if LineBuffer[0] != "SEGMENTS":
                    print("Error: no SEGMENTS label found")
                    exit()
                segmentCount = int(LineBuffer[1])
                for seg in range(segmentCount):
                    LineBuffer = filein.readline().strip()
                    matches = re.findall(r'\((\d+),\s*(\d+)\)', LineBuffer)

                    x1, y1 = map(int, matches[0])
                    x2, y2 = map(int, matches[1])

                    circle = plt.Circle((GRID_MUL*x1, GRID_MUL*y1), radius=POINT_RAIDUS, color=color, fill=True)
                    ax.add_patch(circle)
                    circle = plt.Circle((GRID_MUL*x2, GRID_MUL*y2), radius=POINT_RAIDUS, color=color, fill=True)
                    ax.add_patch(circle)
                    plt.plot([GRID_MUL*x1, GRID_MUL*x2], [GRID_MUL*y1, GRID_MUL*y2], color=color, linewidth=LINE_WIDTH)

                
                # Render the leftout points
                LineBuffer = filein.readline().strip().split()
                if LineBuffer[0] != "POINTS":
                    print("Error: no POINTS label found")
                    exit()
                pointCount = int(LineBuffer[1])
                for pt in range(pointCount):
                    LineBuffer = filein.readline().strip()
                    x, y = map(int, LineBuffer.strip("()").split(","))
                    circle = plt.Circle((GRID_MUL*x, GRID_MUL*y), radius=POINT_RAIDUS, color=color, fill=True)
                    ax.add_patch(circle)
            
            
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