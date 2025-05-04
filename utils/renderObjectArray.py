import sys
sys.path.append("./lib/")
from argparse import ArgumentParser
import re
import queue

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches

from distinctColours import ColourGenerator



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
    "CHIPLET": "#B8B8B8",  # Medium Gray (Darker for better contrast)
    "EMPTY": "none",
    "SIGNAL": "#A0A0A0",  # Gray
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
    "OBSTACLE": "#888888"
}

if __name__ == '__main__':
    
    parser = ArgumentParser(description="Visualise ObjectArray. Possible for pin array, power grid array or showing both")
    parser.add_argument("-i", "--input", required=True, help="Path to input file (required).")
    parser.add_argument("-o", "--output", required=False, help="Path to output image file (optional).")
    parser.add_argument("--dpi", type=int, default=400, required=False, help="DPI for output image (default: 400).")
    parser.add_argument("-p", "--pinSize", type=float,  default=1, required=False, help="Enlarge pin visulisation size (1 for original size)")
    parser.add_argument("-v", "--verbose", action="store_true", help="Enable verbose mode.")
    parser.add_argument("--noLegend", action="store_true", help="Legend is omitted.")
    parser.add_argument("--noTitle", action="store_true", help="Title is omitted.")
    args = parser.parse_args()


    FIG_DPI = args.dpi
    PIN_ENLARGEMENT = 1.0
    
    # start rendering progress
    try:
        with open(args.input, 'r') as filein:
            renderMode = ""
            # Read the first Line to determine the render mode
            LineBuffer = filein.readline().strip().split()
            
            if(((LineBuffer[0] == "PIN") or (LineBuffer[0] == "GRID") or (LineBuffer[0] == "GRID_PIN") or (LineBuffer[0] == "PIN_GRID_PIN") or (LineBuffer[0] == "MICROBUMP")) and (LineBuffer[1] == "VISUALISATION")):
                renderMode = LineBuffer[0]
            else:
                print(f"[RenderPinMap]Error: Unknown Render Mode {LineBuffer[0]} {LineBuffer[1]}")
                sys.exit()

            if args.verbose:
                print(CYAN,"IRISLAB Object Array Rendering Program ", COLORRST)
                
                if renderMode == "PIN": print("Mode: ", GREEN, "Pin Display mode", COLORRST)
                elif renderMode == "GRID": print("Mode: ", GREEN, "Grid Display mode", COLORRST)
                elif renderMode == "GRID_PIN": print("Mode: ", GREEN, "Grid with Pin Display mode", COLORRST)
                elif renderMode == "MICROBUMP": print("Mode: ", GREEN, "MircoBump Display mode", COLORRST)
                else: print("Mode: ", GREEN, "Grid with 2 Pins Display mode", COLORRST)

                print("Input File: ", GREEN, args.input, COLORRST)
                
                if args.output is None:
                    print("Output File: ", RED, "Not saved", COLORRST)
                else:
                    print("Output File: ", GREEN, args.output, COLORRST)
            
                print("Output Image dpi: ", GREEN, FIG_DPI, COLORRST)
                
                if args.pinSize <= 0.0:
                    PIN_ENLARGEMENT = 1.0
                else:
                    PIN_ENLARGEMENT = args.pinSize

                print("PinSize: ", GREEN, 100*(PIN_ENLARGEMENT), "% compared to original" , COLORRST)
                
                
                if args.noLegend:
                    print("Legend display: ", RED, "off", COLORRST)
                else:
                    print("Legend: ", GREEN, "on", COLORRST)
                    
                if args.noTitle:
                    print("Title display: ", RED, "off", COLORRST)
                else:
                    print("Title display: ", GREEN, "on", COLORRST)
            

            LineBuffer = filein.readline().strip().split()
            pitch = int(LineBuffer[0])
            pinRadius = int(LineBuffer[1])
            gridWidth = int(LineBuffer[2])
            gridHeight = int(LineBuffer[3])
            pinWidth = int(LineBuffer[4])
            pinHeight = int(LineBuffer[5])


            GRID_MUL = 10
            PIN_RAD = GRID_MUL*PIN_ENLARGEMENT*float(pinRadius) / (2*float(pitch))
            print(f"Pin pad is set to: {PIN_RAD}")
            if PIN_RAD >= (GRID_MUL/2.0): 
                PIN_RAD = GRID_MUL/2.0

            fig, ax = plt.subplots(figsize=(16, 16))
            ax.set_xlim(0, GRID_MUL*(gridWidth+2))
            ax.set_ylim(0, GRID_MUL*(gridHeight+2))
            ax.set_aspect('equal')
            ax.set_xticks([])
            ax.set_yticks([])
            ax.spines['top'].set_color('white')
            ax.spines['right'].set_color('white')
            ax.spines['bottom'].set_color('white')
            ax.spines['left'].set_color('white')
                
            # Draw the grids
            if((renderMode == "GRID") or (renderMode == "GRID_PIN") or (renderMode == "PIN_GRID_PIN")):
                for j in range(gridHeight):
                    for i in range(gridWidth):
                        LineBuffer = filein.readline().strip().split()
                        color = SIGNAL_COLORS[LineBuffer[2]]
                        # Draw the box
                        rect = plt.Rectangle((GRID_MUL*(i+1), GRID_MUL*(j+1)), GRID_MUL, GRID_MUL, facecolor=color, edgecolor='black', linewidth=1.5, alpha=0.5)
                        ax.add_patch(rect)

            # Draw the Circular pins
            if((renderMode == "PIN") or (renderMode == "GRID_PIN") or (renderMode == "MICROBUMP")):
                for j in range(pinHeight):
                    for i in range(pinWidth):
                        LineBuffer = filein.readline().strip().split()
                        color = SIGNAL_COLORS[LineBuffer[2]]
                        # Draw the box
                        circle = patches.Circle((GRID_MUL*(i+1), GRID_MUL*(j+1)), PIN_RAD, edgecolor=(0, 0, 0, 0.2), facecolor=color)
                        ax.add_patch(circle)
            
            if renderMode == "PIN_GRID_PIN":
                # draw the upper circle first
                for j in range(pinHeight):
                    for i in range(pinWidth):
                        LineBuffer = filein.readline().strip().split()
                        color = SIGNAL_COLORS[LineBuffer[2]]
                        upper_circle = patches.Wedge(
                            center=(GRID_MUL*(i+1), GRID_MUL*(j+1)), 
                            r=PIN_RAD, 
                            theta1=0, 
                            theta2=180,
                            edgecolor='black',
                            facecolor=color
                        )
                        ax.add_patch(upper_circle)
                # draw the lower circle
                for j in range(pinHeight):
                    for i in range(pinWidth):
                        LineBuffer = filein.readline().strip().split()
                        color = SIGNAL_COLORS[LineBuffer[2]]
                        upper_circle = patches.Wedge(
                            center=(GRID_MUL*(i+1), GRID_MUL*(j+1)), 
                            r=PIN_RAD, 
                            theta1=180, 
                            theta2=360,
                            edgecolor='black',
                            facecolor=color
                        )
                        ax.add_patch(upper_circle)

            if renderMode == "MICROBUMP":
                LineBuffer = filein.readline().strip()
                wholeLineBuffer = LineBuffer
                LineBuffer = LineBuffer.split()
                chipletCount = -0
                if(LineBuffer[0] != "CHIPLETS"):
                    print(f"[RenderPinMap]Error: Missing CHIPLET label in Microbump rendering mode: {wholeLineBuffer}")
                chipletCount = int(LineBuffer[1])
                
                for cidx in range(chipletCount):
                    LineBuffer = filein.readline().strip().split()
                    llx = (int(LineBuffer[2]) + 0.5) * GRID_MUL 
                    lly = (int(LineBuffer[3]) + 0.5) * GRID_MUL
                    width = int(LineBuffer[4]) * GRID_MUL
                    height = int(LineBuffer[5]) * GRID_MUL

                    rect = plt.Rectangle((llx, lly), width, height, facecolor='none', edgecolor='black', linewidth=1)
                    ax.add_patch(rect)


                


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




