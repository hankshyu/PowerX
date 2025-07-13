import sys
sys.path.append("./lib/")
from argparse import ArgumentParser
import re
import queue

import numpy as np
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

class CellLabel:
    def __init__(self, celltype, signaltype, label):
        self.celltype = celltype
        self.signaltype = signaltype
        self.label = label

def parse_arguments():
    parser = ArgumentParser(description="Visualise DiffusionEngine.")
    parser.add_argument("-i", "--input", required=True, help="Path to input file (required).")
    parser.add_argument("-o", "--output", required=False, help="Path to output image file (optional).")
    parser.add_argument("--dpi", type=int, default=400, required=False, help="DPI for output image (default: 400).")
    parser.add_argument("-v", "--verbose", action="store_true", help="Enable verbose mode.")
    return parser.parse_args()
    


if __name__ == '__main__':

    args = parse_arguments()
    fig, ax = plt.subplots(figsize=(18, 18))
    
    BORDER_WIDTH = 2

    SKIRT_BORDER = 12
    GRID_MUL = 80

    canvasWidth = -1
    canvasHeight = -1
    canvasLayer = -1

    try:
        with open(args.input, 'r') as filein:
            renderMode = ""
            # Read the first Line to determine the render mode
            LineBuffer = filein.readline().strip().split()

            if((LineBuffer[0] == "DIFFUSION_ENGINE") and ((LineBuffer[1] == "METAL") or (LineBuffer[1] == "VIA")) and (LineBuffer[2] == "VISUALISATION")):            
                renderMode = LineBuffer[1]
                canvasLayer = int(LineBuffer[3])
                canvasWidth = int(LineBuffer[4])
                canvasHeight = int(LineBuffer[5])
            else:
                print(f"[RenderDiffusionEngine]Error: Unknown Render Mode {LineBuffer[0]} {LineBuffer[2]}")
                sys.exit()

            # display the arguments if user specifies verbose mode
            if args.verbose:
                print(CYAN,"IRISLAB Diffusion Engine Rendering Program ", COLORRST)
                print("Render Mode: ", GREEN, renderMode, COLORRST)
                print("Input File: ", GREEN, args.input, COLORRST)
                
                if args.output is None:
                    print("Output File: ", RED, "Not saved", COLORRST)
                else:
                    print("Output File: ", GREEN, args.output, COLORRST)
                
                if args.dpi is None:
                    print("Output Image dpi: ", GREEN, "400", COLORRST)
                else:
                    print("Output Image dpi: ", GREEN, args.dpi, COLORRST)

            ax.set_xlim(0, GRID_MUL*canvasWidth)
            ax.set_ylim(0, GRID_MUL*canvasHeight)
            ax.set_aspect('equal')
            ax.set_xticks([])
            ax.set_yticks([])
            
            # Make plot border thicker
            for spine in ax.spines.values():
                spine.set_linewidth(BORDER_WIDTH)  # You can increase this value

            # start reading each cell
            while True:
                line = filein.readline()
                if not line:  # end of file
                    break
                LineBuffer = line.strip().split()
                if LineBuffer[0] != "Cell":
                    print(f"[RenderDiffusionEngine]Error: Unrecognized Metal Cell position: ")
                    sys.exit()
                
                cellLayer = int(LineBuffer[1])
                cellX = int(LineBuffer[2])
                cellY = int(LineBuffer[3])

                if (cellLayer != canvasLayer) or (cellX < 0 or cellX >= canvasWidth) or (cellY < 0 or cellY >= canvasWidth):
                    print(f"[RenderDiffusionEngine]Error: Metal Cell Position (l = {cellLayer}, x = {cellX}), y = {cellY} not in Metal Layer (l = {canvasLayer}, width = {canvasWidth}, height = {canvasHeight})")
                    sys.exit()

                cellType = ""
                cellSt = ""
                cellIndex = ""
                
                line = filein.readline().strip()
                pattern = (
                    r"celltype\s*=\s*(\S+)\s+"
                    r"signaltype\s*=\s*(\S+)\s+"
                    r"label\s*=\s*(-?\d+)"
                )
                match = re.fullmatch(pattern, line)
                if match:
                    cellType = match.group(1)
                    cellSt = match.group(2)
                    cellIndex = int(match.group(3))
                else:
                    print(f"[RenderDiffusionEngine]Error: Metal Cell Attribute mismatch {line}")
                
                labelCount = 0
                line = filein.readline().strip()

                match = re.match(r"labels\((\d+)\):", line)
                if match:
                    labelCount = int(match.group(1))
                    # print(f"Valid label index: {labelCount}")
                else:
                    print(f"[RenderDiffusionEngine]Error: Metal Cell labels count error {line}")

                for labelIdx in range(labelCount):
                    print("todo parse iex")

                # Start rendering cell according to attributes
                # use the skirt

                skirtColor = "none"
                if(cellType == "CellType::OBSTACLES"):
                    skirtColor = SIGNAL_COLORS["OBSTACLE"]
                elif(cellType == "CellType::PREPLACED"):
                    skirtColor = "#FF0000"
                elif(cellType == "CellType::MARKED"):
                    skirtColor = "#000000"

                skirtRect = plt.Rectangle(
                    (GRID_MUL*(cellX), GRID_MUL*(cellY)),
                    GRID_MUL, GRID_MUL, 
                    facecolor=skirtColor,
                    edgecolor='black',
                    linewidth=1,
                    alpha=0.75
                )

                ax.add_patch(skirtRect)

                centreRect = plt.Rectangle(
                    (GRID_MUL*(cellX) + SKIRT_BORDER, GRID_MUL*(cellY) + SKIRT_BORDER),
                    GRID_MUL - 2*SKIRT_BORDER, GRID_MUL - 2*SKIRT_BORDER,
                    facecolor=SIGNAL_COLORS[cellSt],
                    linewidth=0,
                    alpha=0.9
                )
                ax.add_patch(centreRect)

                label_char = ""
                if cellType == "CellType::PREPLACED":
                    label_char = "P"
                elif cellType == "CellType::MARKED":
                    label_char = "M"

                if label_char:
                    ax.text(
                        GRID_MUL * (cellX + 1) - GRID_MUL/4 - 12,
                        GRID_MUL * (cellY + 1) - GRID_MUL/4 - 8,
                        label_char,
                        fontsize=1,
                        color='black',
                        ha='left',
                        va='top',
                        # weight='bold'
                    )
                # for debug
                # break
            
            if args.output is not None:
                plt.savefig(args.output, dpi=args.dpi, bbox_inches='tight', pad_inches=0.2)
                plt.close()
            else:
                plt.show()
                
               

                
            


    except FileNotFoundError:
        print(f"[RenderPinMap]Error: File \"{args.input}\" not found")
        sys.exit()
    except PermissionError:
        print(f"[RenderPinMap]Error: Permission denied when accessing \"{args.input}\"")
        sys.exit()