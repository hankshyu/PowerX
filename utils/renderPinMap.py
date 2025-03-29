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
PIN_COLORS = {
    "CHIPLET": "#B8B8B8",  # Medium Gray (Darker for better contrast)
    "EMPTY": "none",
    "SIGNAL": "#A0A0A0",  # Gray
    "GROUND": "#5A773D",  # Muted Olive Green (Adjusted for better differentiation)
    "POWER_1": "#B04E3A",  # Muted Brick Red
    "POWER_2": "#3B6B8E",  # Steel Blue
    "POWER_3": "#9B8832",  # Desaturated Gold
    "POWER_4": "#6B4C9A",  # Deep Purple (Strong but not too vibrant, contrasts well)
}

if __name__ == '__main__':
    
    parser = ArgumentParser(description="visualise pinout files (bumpmap or Pinouts, uBumps and C4)")
    
    parser.add_argument("-i", "--input", required=True, help="Input chiplet file path")
    parser.add_argument("-o", "--output", required=False, help="Output image file path")
    parser.add_argument("--dpi", required=False, help="Adjust Image dpi value (defulat = 400)")
    parser.add_argument("-p", "--pinSize", required=False, help="Enlarge pin visulisation size (1 for original size)")
    parser.add_argument("-v", "--verbose", action = "store_true", help="Disable Legend Rendering")
    parser.add_argument("--noLegend", action = "store_true", help="Disable Legend Rendering")
    parser.add_argument("--noTitle", action = "store_true", help="Disable Title Rendering")
    args = parser.parse_args()
    
    
    FIG_DPI = 400
    PIN_ENLARGEMENT = 1.0
    
    if args.verbose:
        print(CYAN,"IRISLAB PinMap Rendering Program ", COLORRST)
        print("Input File: ", GREEN, args.input, COLORRST)
        
        if args.output is None:
            print("Output File: ", RED, "Not saved", COLORRST)
        else:
            print("Output File: ", GREEN, args.output, COLORRST)
        
        if args.dpi is None:
            print("Output Image dpi = ", GREEN, "400", COLORRST)
        else:
            FIG_DPI = int(args.dpi)
            print("Output Image dpi = ", GREEN, FIG_DPI, COLORRST)
        
        if args.pinSize is None or float(args.pinSize) == 1.0 or float(args.pinSize) <= 0.0:
            PIN_ENLARGEMENT = 1.0
            print("PinSize: ", GREEN, "Original Size", COLORRST)
        elif float(args.pinSize) > 1.0: 
            PIN_ENLARGEMENT = float(args.pinSize)
            print("PinSize: ", GREEN, 100*(PIN_ENLARGEMENT), "% compared to original" , COLORRST)
        
        
        if args.noLegend:
            print("Legend display: ", RED, "off", COLORRST)
        else:
            print("Legend: ", GREEN, "on", COLORRST)
            
        if args.noTitle:
            print("Title display: ", RED, "off", COLORRST)
        else:
            print("Title display: ", GREEN, "on", COLORRST)
            

    # start rendering progress
    try:
        with open(args.input, 'r') as filein:
            renderMode = ""
            # Read the first Line to determine the render mode
            LineBuffer = filein.readline().strip().split()
            if((LineBuffer[0] == "BUMPMAP") or (LineBuffer[0] == "UBUMP") or LineBuffer[0] == "BALLOUT") and (LineBuffer[1] == "VISUALISATION"):
                renderMode = LineBuffer[0]
            else:
                print(f"[RenderPinMap]Error: Unknown Render Mode {LineBuffer[0]} {LineBuffer[1]}")
                sys.exit()
            
            # Read the second line to get chiplet name and dimensions
            LineBuffer = filein.readline().strip().split()
            Pinout_name = LineBuffer[0]  # Extract chiplet name
            Pinout_h = int(LineBuffer[1])
            pinout_w = int(LineBuffer[2])
            
            LineBuffer = filein.readline().strip().split()
            Interposer_width = int(LineBuffer[0])
            Interposer_height = int(LineBuffer[1])
                        
            scale_factor = (Interposer_width + Interposer_height) / 2400.0
            
            # Dynamically adjust all figure parameters according to scale factor
            legend_rect_size = int(20 * scale_factor)
            legend_spacing = int(35 * scale_factor)
            legend_margin = int(50 * scale_factor)
            FIG_SIZE = (12, 12)


            # rendering Outermost contour (footprint)        
            fig, ax = plt.subplots(figsize=FIG_SIZE)
            if args.noLegend:
                ax.set_xlim(-Interposer_width*0.1, Interposer_width * 1.1)
                ax.set_ylim(0, Interposer_height * 1.05)
                
            else:
                ax.set_xlim(-0.3 * Interposer_width, Interposer_width * 1.3)
                ax.set_ylim(-Interposer_height*0.1, Interposer_height * 1.1)
            
            ax.set_aspect('equal', adjustable='datalim')
                
            ax.spines['top'].set_color('white')
            ax.spines['right'].set_color('white')
            ax.spines['bottom'].set_color('white')
            ax.spines['left'].set_color('white')
        
            rect = patches.Rectangle((0, 0), Interposer_width, Interposer_height, linewidth=1, edgecolor='black', facecolor=PIN_COLORS["CHIPLET"])
            ax.add_patch(rect)
            
            # render chiplets if in uBump mode
            if renderMode == "UBUMP":
                LineBuffer = filein.readline().strip().split()
                if(LineBuffer[0] != "CHIPLETS"):
                    print(f"[RenderPinMap]Error: No CHIPLET section present")
                    exit()

                chiplets = int(LineBuffer[1])
                for i in range(chiplets):
                    LineBuffer = filein.readline().strip().split()
                    llx = int(LineBuffer[2])
                    lly = int(LineBuffer[3])
                    width = int(LineBuffer[4])
                    height = int(LineBuffer[5])
                    rect = patches.Rectangle((llx, lly), width, height, linewidth=1, edgecolor='black', facecolor='none')
                    ax.add_patch(rect)
            
            # Draw the pins 
            LineBuffer = filein.readline().strip().split()
            if(LineBuffer[0] != "PINS"):
                print(f"[RenderPinMap]Error: No PINS section present")
                exit()
            pinCount = int(LineBuffer[1])
            for i in range(pinCount):
                LineBuffer = filein.readline().strip().split()
                centreX = int(LineBuffer[0])
                centreY = int(LineBuffer[1])
                radius = int(LineBuffer[2]) * PIN_ENLARGEMENT
                pin_definition = LineBuffer[3]
                color = PIN_COLORS["EMPTY"] # Default to blue for unclassified signals
                circle_edge_color = (0, 0, 0, 0.2)
                
                
                pinColorLookUp = PIN_COLORS.get(pin_definition)
                if pinColorLookUp is not None:
                    color = pinColorLookUp
                    if pin_definition != "SIGNAL" and pin_definition != "OBSTACLE" and pin_definition != "EMPTY":
                        circle_edge_color = 'black'
                else:
                    print(f"[RenderPinMap]Warning: {pin_definition} is unseen pin type, set to unknown")
                
                circle = patches.Circle((centreX, centreY), radius, edgecolor=circle_edge_color, facecolor=color)
                ax.add_patch(circle)
            
            
            # Draw legend items
            legend_x = Interposer_width + legend_margin
            legend_y = legend_margin + (len(PIN_COLORS) + 5) * legend_spacing

            for i, (label, color) in enumerate(PIN_COLORS.items()):
                y_pos = legend_y + legend_rect_size - i * legend_spacing

                # Add color rectangle first
                rect = patches.Rectangle(
                    (legend_x, y_pos),
                    legend_rect_size, legend_rect_size,
                    edgecolor='black', facecolor=color, linewidth=1.5
                )
                ax.add_patch(rect)

                # Add text next
                ax.text(
                    legend_x + legend_rect_size + 10 * scale_factor,
                    y_pos + legend_rect_size / 2,  # Center vertically
                    label, verticalalignment='center', fontsize=13)           
            
            # display title
            if not args.noTitle:
                ax.set_title(f"{Pinout_name} {pinout_w} x {Pinout_h}", fontsize = 25, fontweight='bold')
                
            # Add title
            if args.output is not None:
                plt.savefig(args.output, dpi=FIG_DPI)  # Remove extra padding
            
            plt.close()
                
    except FileNotFoundError:
        print(f"[RenderPinMap]Error: File \"{args.input}\" not found")
        sys.exit()
    except PermissionError:
        print(f"[RenderPinMap]Error: Permission denied when accessing \"{args.input}\"")
        sys.exit()

        
        
    
