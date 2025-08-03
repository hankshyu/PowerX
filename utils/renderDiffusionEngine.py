import sys
sys.path.append("./lib/")
from argparse import ArgumentParser
import re
import queue

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches

from collections import defaultdict

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
    "OBSTACLE": "#B8B8B8"
}

SIGNAL_LABEL_COLORS = {
    "EMPTY": "none",
    "POWER_1": "#1e54b0",
    "POWER_2": "#e66022",
    "POWER_3": "#ffea00",
    "POWER_4": "#4b3575",
}

SKIRT_COLORS = {
    "CellType::CANDIDATE" : "#FFC1CC",
    "CellType::OBSTACLES" : SIGNAL_COLORS["OBSTACLE"],
    "CellType::PREPLACED" : "#FF0000",
    "CellType::MARKED"    : "#00FF00",
    "CellType::EMPTY"    : "#FFFFFF",
}
# hyperparameters

BORDER_WIDTH = 2

SKIRT_BORDER = 12
GRID_MUL = 80

VIA_SIZE = 60

canvasWidth = -1
canvasHeight = -1
metalLayer = -1
viaLayer = -1

metalCellCount = -1
viaCellCount = -1


class CellLabelObj:
    def __init__(self, label, signaltype, count):
        self.label = label
        self.signaltype = signaltype
        self.count = count

def parse_arguments():
    parser = ArgumentParser(description="Visualise DiffusionEngine.")
    parser.add_argument("-i", "--input", required=True, help="Path to input file (required).")
    parser.add_argument("-o", "--output", required=False, help="Path to output image file (optional).")
    parser.add_argument("--dpi", type=int, default=400, required=False, help="DPI for output image (default: 400).")
    parser.add_argument("-v", "--verbose", action="store_true", help="Enable verbose mode.")
    return parser.parse_args()

def parseCell(filein, metalOrVia):

    # cell Atrributes
    cellLayer = -1
    cellX = -1
    cellY = -1
    cellType = ""
    cellSt = ""
    cellLabel = ""
    cellIdxLabels = []


    LineBuffer = filein.readline().strip().split()
    if LineBuffer[0] != "Cell":
        print(f"[RenderDiffusionEngine]Error: Unrecognized {metalOrVia} Cell position: ")
        sys.exit()
    
    cellLayer = int(LineBuffer[1])
    cellX = int(LineBuffer[2])
    cellY = int(LineBuffer[3])

    if metalOrVia == "Metal":
        canvasLayer = metalLayer
    else:
        canvasLayer = viaLayer

    if (cellLayer != canvasLayer) or (cellX < 0 or cellX >= canvasWidth) or (cellY < 0 or cellY >= canvasWidth):
        print(f"[RenderDiffusionEngine]Error: {metalOrVia} Cell Position (l = {cellLayer}, x = {cellX}, y = {cellY}) not in  Layer (l = {canvasLayer}, width = {canvasWidth}, height = {canvasHeight})")
        sys.exit()

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
        cellLabel = int(match.group(3))
    else:
        print(f"[RenderDiffusionEngine]Error: {metalOrVia} Cell Attribute mismatch {line}")
    
    labelCount = 0
    line = filein.readline().strip()

    match = re.match(r"labels\((\d+)\):", line)
    if match:
        labelCount = int(match.group(1))
        # print(f"Valid label index: {labelCount}")
    else:
        print(f"[RenderDiffusionEngine]Error: {metalOrVia} Cell labels count error {line}")

    for labelIdx in range(labelCount):
        LineBuffer = filein.readline().strip().split()
        cellIdxLabels.append(CellLabelObj(int(LineBuffer[0]), LineBuffer[1], int(LineBuffer[2])))
    
    # sort in descending order
    cellIdxLabels.sort(key=lambda x: x.count, reverse=True)


    return cellLayer, cellX, cellY, cellType, cellSt, cellLabel, cellIdxLabels 

if __name__ == '__main__':

    args = parse_arguments()
    fig, ax = plt.subplots(figsize=(18, 18))
    
    try:
        with open(args.input, 'r') as filein:
            renderMode = ""

            # Read the first Line to determine the render mode
            LineBuffer = filein.readline().strip().split()

            if((LineBuffer[0] == "DIFFUSION_ENGINE") and ((LineBuffer[1] == "METAL") or (LineBuffer[1] == "VIA") or (LineBuffer[1] == "METAL_AND_VIA")) and (LineBuffer[2] == "VISUALISATION")):   
                renderMode = LineBuffer[1]
                metalLayer = int(LineBuffer[3])
                viaLayer = int(LineBuffer[4])
                canvasWidth = int(LineBuffer[5])
                canvasHeight = int(LineBuffer[6])
            else:
                print(f"[RenderDiffusionEngine]Error: Unknown Render Mode {LineBuffer[0]} {LineBuffer[2]}")
                sys.exit()
            
            # Read the second Line to determine cell counts (metal/via)
            line = filein.readline()
            LineBuffer = line.strip().split()
            if(LineBuffer[0] != "METAL_CELLCOUNT") or (LineBuffer[2] != "VIA_CELLCOUNT"):
                print(f"[RenderDiffusionEngine]Error: Unknown metal/via count: {line}")
                sys.exit()

            metalCellCount = int(LineBuffer[1])
            viaCellCount = int(LineBuffer[3])
            

            # display the arguments if user specifies verbose mode
            if args.verbose:
                print(CYAN,"IRISLAB Diffusion Engine Rendering Program ", COLORRST)
                if renderMode == "METAL":
                    print("Render Mode: ", GREEN, "Metal Layer ", metalLayer, COLORRST)
                elif renderMode == "VIA":
                    print("Render Mode: ", GREEN, "Via Layer ", viaLayer, COLORRST)
                elif renderMode == "METAL_AND_VIA":
                    print("Render Mode: ", GREEN, "Metal Layer ", metalLayer, " with Via Layer ", viaLayer, COLORRST)

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

            # Render Metal Cells
            if renderMode == "METAL" or renderMode == "METAL_AND_VIA":
                # start reading each cell
                for renderMetalCellIdx in range(metalCellCount):

                    # cell Atrributes
                    cellLayer, cellX, cellY, cellType, cellSt, cellLabel, cellIdxLabels = parseCell(filein, "Metal")
                    # Start rendering cell according to attributes
                    # use the skirt

                    skirtRect = plt.Rectangle(
                        (GRID_MUL*(cellX), GRID_MUL*(cellY)),
                        GRID_MUL, GRID_MUL, 
                        facecolor=SKIRT_COLORS[cellType],
                        edgecolor='black',
                        linewidth=0.5,
                        alpha=0.8
                    )
                    ax.add_patch(skirtRect)
                    if cellType == "CellType::CANDIDATE":
                        coreCentreColor = SKIRT_COLORS[cellType]
                    else:
                        coreCentreColor = SIGNAL_COLORS[cellSt]

                    coreCentreAlpha = 0.9

                    if cellSt == "EMPTY" and len(cellIdxLabels) != 0:
                        coreCentreAlpha = 0.6
                        signaltype_sum = defaultdict(int)
                        for obj in cellIdxLabels:
                            signaltype_sum[obj.signaltype] += obj.count

                        # Find signaltype with largest total
                        max_signaltype = max(signaltype_sum.items(), key=lambda x: x[1])
                        coreCentreColor = SIGNAL_COLORS[max_signaltype[0]]


                    centreRect = plt.Rectangle(
                        (GRID_MUL*(cellX) + SKIRT_BORDER, GRID_MUL*(cellY) + SKIRT_BORDER),
                        GRID_MUL - 2*SKIRT_BORDER, GRID_MUL - 2*SKIRT_BORDER,
                        facecolor=coreCentreColor,
                        linewidth=0,
                        alpha=coreCentreAlpha
                    )
                    ax.add_patch(centreRect)

                    cellType_char = ""
                    if cellType == "CellType::PREPLACED":
                        cellType_char = "P"
                    elif cellType == "CellType::MARKED":
                        cellType_char = "M"

                    if cellType_char:
                        ax.text(
                            GRID_MUL * (cellX+1) - VIA_SIZE/2,
                            GRID_MUL * (cellY+1) - VIA_SIZE/2,
                            cellType_char,
                            fontsize=1,
                            color='black',
                            ha='right',
                            va='top',
                        )
                    
                    if (cellLabel >= 0):
                        ax.text(
                            GRID_MUL * (cellX+1) - VIA_SIZE/2,
                            GRID_MUL * (cellY) + VIA_SIZE/2,
                            cellLabel,
                            fontsize=1,
                            color='black',
                            ha='right',
                            va='bottom',
                        )
                    
                    if len(cellIdxLabels) != 0:
                        lbword = f"{cellIdxLabels[0].label}:{cellIdxLabels[0].count}"
                        lbcolor = SIGNAL_LABEL_COLORS[cellIdxLabels[0].signaltype]
                        ax.text(
                            GRID_MUL * (cellX) + VIA_SIZE/2 - 5,
                            GRID_MUL * (cellY+1) - VIA_SIZE/2,
                            lbword,
                            fontsize=0.6,
                            color=lbcolor,
                            ha='left',
                            va='top',
                        )
                    if len(cellIdxLabels) >= 2:
                        lbword = f"{cellIdxLabels[1].label}:{cellIdxLabels[1].count}"
                        lbcolor = SIGNAL_LABEL_COLORS[cellIdxLabels[1].signaltype]
                        ax.text(
                            GRID_MUL * (cellX) + VIA_SIZE/2 - 5,
                            GRID_MUL * (cellY+1) - VIA_SIZE/2 - 10,
                            lbword,
                            fontsize=0.6,
                            color=lbcolor,
                            ha='left',
                            va='top',
                        )
                    

            # Render Via Cells
            if renderMode == "VIA" or renderMode == "METAL_AND_VIA":
                # start reading each cell
                for renderViaCellIdx in range(viaCellCount):

                    # cell Atrributes
                    cellLayer, cellX, cellY, cellType, cellSt, cellLabel, cellIdxLabels = parseCell(filein, "Via")
                    # Start rendering cell according to attributes
                    # use the skirt

                    viaOffSet = GRID_MUL - (VIA_SIZE/2)
                    viaSkirtBorder = SKIRT_BORDER/2

                    skirtRect = plt.Rectangle(
                        (GRID_MUL*(cellX) + viaOffSet, GRID_MUL*(cellY) + viaOffSet),
                        VIA_SIZE, VIA_SIZE, 
                        facecolor=SKIRT_COLORS[cellType],
                        edgecolor='black',
                        linewidth=0.5,
                        alpha=0.8
                    )
                    ax.add_patch(skirtRect)

                    if cellType == "CellType::CANDIDATE":
                        coreCentreColor = SKIRT_COLORS[cellType]
                    else:
                        coreCentreColor = SIGNAL_COLORS[cellSt]

                    coreCentreAlpha = 0.9

                    if cellSt == "EMPTY" and len(cellIdxLabels) != 0:
                        coreCentreAlpha = 0.6
                        signaltype_sum = defaultdict(int)
                        for obj in cellIdxLabels:
                            signaltype_sum[obj.signaltype] += obj.count

                        # Find signaltype with largest total
                        max_signaltype = max(signaltype_sum.items(), key=lambda x: x[1])
                        coreCentreColor = SIGNAL_COLORS[max_signaltype[0]]
                    
                    centreRect = plt.Rectangle(
                        (GRID_MUL*(cellX) + viaOffSet + viaSkirtBorder , GRID_MUL*(cellY) + viaOffSet + viaSkirtBorder),
                        VIA_SIZE - 2*viaSkirtBorder, VIA_SIZE - 2*viaSkirtBorder,
                        facecolor=coreCentreColor,
                        linewidth=0,
                        alpha=coreCentreAlpha
                    )
                    ax.add_patch(centreRect)

                    cellType_char = ""
                    if cellType == "CellType::PREPLACED":
                        cellType_char = "P"
                    elif cellType == "CellType::MARKED":
                        cellType_char = "M"

                    if cellType_char:
                        ax.text(
                            GRID_MUL * cellX + viaOffSet + VIA_SIZE - viaSkirtBorder - 2,
                            GRID_MUL * cellY + viaOffSet + VIA_SIZE - viaSkirtBorder - 2,
                            cellType_char,
                            fontsize=1,
                            color='black',
                            ha='right',
                            va='top',
                        )

                    if (cellLabel >= 0):
                        ax.text(
                            GRID_MUL * cellX + viaOffSet + VIA_SIZE - viaSkirtBorder - 2,
                            GRID_MUL * cellY + viaOffSet + viaSkirtBorder + 2,
                            cellLabel,
                            fontsize=1,
                            color='black',
                            ha='right',
                            va='bottom',
                        )
                    if len(cellIdxLabels) != 0:
                        lbword = f"{cellIdxLabels[0].label}:{cellIdxLabels[0].count}"
                        lbcolor = SIGNAL_LABEL_COLORS[cellIdxLabels[0].signaltype]
                        ax.text(
                            GRID_MUL * cellX + viaOffSet + viaSkirtBorder + 2,
                            GRID_MUL * cellY + viaOffSet + VIA_SIZE - viaSkirtBorder - 2,
                            lbword,
                            fontsize=0.6,
                            color=lbcolor,
                            ha='left',
                            va='top',
                        )
                    if len(cellIdxLabels) >= 2:
                        lbword = f"{cellIdxLabels[1].label}:{cellIdxLabels[1].count}"
                        lbcolor = SIGNAL_LABEL_COLORS[cellIdxLabels[1].signaltype]
                        ax.text(
                            GRID_MUL * cellX + viaOffSet + viaSkirtBorder + 2,
                            GRID_MUL * cellY + viaOffSet + VIA_SIZE - viaSkirtBorder - 12,
                            lbword,
                            fontsize=0.6,
                            color=lbcolor,
                            ha='left',
                            va='top',
                        )



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