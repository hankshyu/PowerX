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

GRID_COLORS = {
    0: "none",
    1: "#1e81b0",
    2: "#e67e22",
    3: "#ffc107",
    4: "#b29dd9",
    5: "#72f2ee",
    6: "#fc83bc",
    7: "#c0392b",
    8: "#21b2ab",
    9: "#b0f294",
    10: "#8d57a3",
    11: "#5cb85c",
    12: "#B3B3B3",
    13: "#292929",
    14: "#ff0000",
    15: "#000000",
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

def print_arguments(args):
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

    if args.verbose:
        print_arguments(args)
        
    fig, ax = plt.subplots(figsize=(24, 24))

    GRID_MUL = 10
    PIN_RAD = 3
    powerPlaneWidth = 0
    powerPlaneHeight = 0
    # start rendering progress
    try:
        with open(args.input, 'r') as filein:
            renderMode = ""
            # Read the first Line to determine the render mode
            LineBuffer = filein.readline().strip().split()
            if((LineBuffer[0] == "M5") or (LineBuffer[0] == "M7")) and (LineBuffer[1] == "VISUALISATION"):
                renderMode = LineBuffer[0]
                powerPlaneWidth = int(LineBuffer[2])
                powerPlaneHeight = int(LineBuffer[3])
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
                    if isinstance(token, int):
                        color = GRID_COLORS[token]
                    elif isinstance(token, list):
                        color = GRID_COLORS[14]
                    else:
                        print(f"Error: Token unrecognized {token}")
                        exit()
                        
                    # Draw the box
                    rect = plt.Rectangle((GRID_MUL*i, GRID_MUL*j), GRID_MUL, GRID_MUL, facecolor=color, edgecolor='black', linewidth=1.5)
                    ax.add_patch(rect)
            
            # Draw the pin
            LineBuffer = filein.readline().strip().split()
            kindsOfSigType = int(LineBuffer[1])
            for types in range(kindsOfSigType):
                LineBuffer = filein.readline().strip().split()
                color = SIGNAL_COLORS[LineBuffer[0]]
                cordCount = int(LineBuffer[1])
                
                xArr, yArr = parse_coordinates(filein.readline(), cordCount)
                for i in range(cordCount):
                    circle = patches.Circle((GRID_MUL*xArr[i], GRID_MUL*yArr[i]), PIN_RAD, edgecolor='black', facecolor=color)
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