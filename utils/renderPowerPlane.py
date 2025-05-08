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
    parser = argparse.ArgumentParser(description="Draw a 2D image using matplotlib from an input file.")
    
    parser.add_argument("-i", "--input", required=True, help="Path to input file (required).")
    parser.add_argument("-o", "--output", help="Path to output image file (optional).")
    parser.add_argument("--dpi", type=int, default=400, help="DPI for output image (default: 400).")
    parser.add_argument("-v", "--verbose", action="store_true", help="Enable verbose mode.")
    parser.add_argument("--noLegend", action="store_true", help="Legend is omitted.")
    parser.add_argument("--noTitle", action="store_true", help="Title is omitted.")
    
    return parser.parse_args()

def parse_line(LineBuffer: str, powerPlaneWidth: int):
    tokens = []
    i = 0
    while i < len(LineBuffer):
        if LineBuffer[i] == '[':
            # Start of an array
            j = i
            bracket_count = 1
            i += 1
            while i < len(LineBuffer) and bracket_count > 0:
                if LineBuffer[i] == '[':
                    bracket_count += 1
                elif LineBuffer[i] == ']':
                    bracket_count -= 1
                i += 1
            array_str = LineBuffer[j:i]
            try:
                array = ast.literal_eval(array_str)
                if isinstance(array, list) and all(isinstance(x, int) for x in array):
                    tokens.append(array)
                else:
                    raise ValueError
            except Exception:
                print(f"Error: Could not parse array token: {array_str}")
                exit()
        elif LineBuffer[i].isdigit() or (LineBuffer[i] == '-' and i + 1 < len(LineBuffer) and LineBuffer[i + 1].isdigit()):
            # Start of an int
            j = i
            while i < len(LineBuffer) and (LineBuffer[i].isdigit() or LineBuffer[i] == '-'):
                i += 1
            tokens.append(int(LineBuffer[j:i]))
        elif LineBuffer[i] in ' ,':
            # Skip delimiters
            i += 1
        else:
            print(f"Error: Unrecognized character at position {i}: {LineBuffer[i]}")
            exit()
            
            i += 1

    if len(tokens) != powerPlaneWidth:
        print(f"Error: Expected {powerPlaneWidth} tokens, but parsed {len(tokens)} tokens.")
        exit()
    
    return tokens

def parse_coordinates(coordString: str, cordCount: int):
    # Use regex to extract all (x, y) pairs
    pattern = r'\(\s*(-?\d+)\s*,\s*(-?\d+)\s*\)'
    matches = re.findall(pattern, coordString)

    xArr = []
    yArr = []

    for x_str, y_str in matches:
        xArr.append(int(x_str))
        yArr.append(int(y_str))

    if len(xArr) != cordCount:
        print(f"Error: Expected {cordCount} coordinates, but found {len(xArr)}.")
        exit()

    return xArr, yArr

if __name__ == "__main__":

    args = parse_arguments()

        
    fig, ax = plt.subplots(figsize=(24, 24))

    GRID_MUL = 10
    PIN_RAD = 4
    powerPlaneWidth = 0
    powerPlaneHeight = 0
    visualiseOverlap = False
    visualiseM5UBump = False
    visualiseM7C4 = False
    # start rendering progress
    try:
        with open(args.input, 'r') as filein:
            renderMode = ""
            # Read the first Line to determine the render mode
            LineBuffer = filein.readline().strip().split()
            if not ((LineBuffer[0] == "PGM5") or (LineBuffer[0] == "PGM7") or (LineBuffer[0] == "PGOVERLAP")):
                print("Error: Render mode unrecognized")
                exit()

            if not (LineBuffer[1] == "VISUALISATION"):
                print("Error: Render mode unrecognized")
                exit()
                
            renderMode = LineBuffer[0] + " " + LineBuffer[1]
            powerPlaneWidth = int(LineBuffer[2])
            powerPlaneHeight = int(LineBuffer[3])

            if not (LineBuffer[4] == "OVERLAP"):
                print("Error: Overlap Render mode unspecified")
                exit()
            visualiseOverlap = (LineBuffer[5] == "1")

            if not (LineBuffer[6] == "M5_UBUMP"):
                print("Error: M5 uBump Rendering unsepcified")
                exit()
            visualiseM5UBump = (LineBuffer[7] == "1")

            if not (LineBuffer[8] == "M7_C4"):
                print("Error: M7 C4 Rendering unsepcified")
                exit()
            visualiseM7C4 = (LineBuffer[9] == "1")
            
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
                print(f"Canvas Size: {powerPlaneWidth} x {powerPlaneHeight}")
                if visualiseOverlap:
                    print("Display Overlap: ", GREEN, "on", COLORRST)
                else:
                    print("Display Overlap: ", RED, "off", COLORRST)

                if visualiseM5UBump:
                    print("Display M5 uBump: ", GREEN, "on", COLORRST)
                else:
                    print("Display M5 uBump: ", RED, "off", COLORRST)

                if visualiseM7C4:
                    print("Display M7 c4 Bump: ", GREEN, "on", COLORRST)
                else:
                    print("Display M7 c4 Bump: ", RED, "off", COLORRST)
                    

            ax.set_xlim(0, GRID_MUL*powerPlaneWidth)
            ax.set_ylim(0, GRID_MUL*powerPlaneHeight)
            ax.set_aspect('equal')
            ax.set_xticks([])
            ax.set_yticks([])
            
            ax.spines['top'].set_color('white')
            ax.spines['right'].set_color('white')
            ax.spines['bottom'].set_color('white')
            ax.spines['left'].set_color('white')
                
            # Draw each box
            for j in range(powerPlaneHeight):
                LineBuffer = filein.readline().strip()
                LineArr = parse_line(LineBuffer, powerPlaneWidth)
                for i in range(powerPlaneWidth):
                    # Generate a random RGB color (each channel between 0 and 1)
                    token = LineArr[i]
                    color = 0
                    alpha = 0.5
                    if isinstance(token, int):
                        if token >= 100:
                            alpha = 0.8
                            color = GRID_COLORS[token - 100]
                        else:
                            alpha = 0.5
                            color = GRID_COLORS[token]
                    elif isinstance(token, list):
                        alpha = 0.7
                        color = GRID_COLORS[14]
                    else:
                        print(f"Error: Token unrecognized {token}")
                        exit()
                        
                    # Draw the box
                    rect = plt.Rectangle((GRID_MUL*i, GRID_MUL*(powerPlaneHeight-j-1)), GRID_MUL, GRID_MUL, facecolor=color, edgecolor='black', linewidth=1.5, alpha = alpha)
                    ax.add_patch(rect)
            
            # Draw the M5 ubumps (marked by the upper circle)
            LineBuffer = filein.readline().strip().split()
            if((LineBuffer[0] != "M5_UBUMP") or (LineBuffer[1] != "SIGNAL_TYPES")):
                print("Error: M5 uBump labels missing")
                exit()
            kindsOfSigType = int(LineBuffer[2])
            for types in range(kindsOfSigType):
                LineBuffer = filein.readline().strip().split()
                color = SIGNAL_COLORS[LineBuffer[0]]
                cordCount = int(LineBuffer[1])
                
                xArr, yArr = parse_coordinates(filein.readline(), cordCount)
                for i in range(cordCount):
                    circle = patches.Wedge(
                        center=(GRID_MUL*xArr[i], GRID_MUL*yArr[i]), 
                        r=PIN_RAD, 
                        theta1=0, 
                        theta2=180,
                        edgecolor='black',
                        facecolor=color
                    )
                    ax.add_patch(circle)
                    
            # Draw the M7 C4 Bumps (marked by the lower circle)
            LineBuffer = filein.readline().strip().split()
            if((LineBuffer[0] != "M7_C4BUMP") or (LineBuffer[1] != "SIGNAL_TYPES")):
                print("Error: M7 C4 Bump labels missing")
                exit()
            kindsOfSigType = int(LineBuffer[2])
            for types in range(kindsOfSigType):
                LineBuffer = filein.readline().strip().split()
                color = SIGNAL_COLORS[LineBuffer[0]]
                cordCount = int(LineBuffer[1])
                
                xArr, yArr = parse_coordinates(filein.readline(), cordCount)
                for i in range(cordCount):
                    circle = patches.Wedge(
                        center=(GRID_MUL*xArr[i], GRID_MUL*yArr[i]), 
                        r=PIN_RAD, 
                        theta1=180, 
                        theta2=360,
                        edgecolor='black',
                        facecolor=color
                    )
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


    



    # Placeholder: Actual drawing logic will go here
    # Example: draw_figure(args.input, args.output, args.dpi, args.verbose)