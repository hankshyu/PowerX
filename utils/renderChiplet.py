import sys
sys.path.append("./lib/")
from argparse import ArgumentParser
import re
import queue

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches

# Define a global mapping for pin colors
PIN_COLORS = {
    "CHIPLET": "#B8B8B8",  # Medium Gray (Darker for better contrast)
    "SIGNAL": "none",  # Transparent
    "GROUND": "#5A773D",  # Muted Olive Green (Adjusted for better differentiation)
    "POWER_1": "#B04E3A",  # Muted Brick Red
    "POWER_2": "#3B6B8E",  # Steel Blue
    "POWER_3": "#9B8832",  # Desaturated Gold
    "POWER_4": "#6B4C9A",  # Deep Purple (Strong but not too vibrant, contrasts well)

}


class Cord:
    def __init__(self, x, y):
        self.x = x
        self.y = y
    def __str__(self) -> str:
        return "({}, {})".format(self.x, self.y)
    def __eq__(self, other):
        if isinstance(other, Cord):
            return ((self.x == other.x) and (self.y == other.y))
        return False
    def __hash__(self) -> int:
        return hash((self.x, self.y))

def parse_chiplet_file(file_path):
    pin_queue = queue.Queue()
    pin_dict = {}
    
    with open(file_path, 'r') as file:
        
        # Read the first line to get chiplet name and dimensions
        first_line = file.readline().strip().split()
        chiplet_name = first_line[0]  # Extract chiplet name
        width, height = map(int, first_line[1:])
        
        # Read the pin definitions
        for line in file:
            match = re.match(r"\((\d+), (\d+)\) (\S+)", line.rstrip())
            if match:
                pin_x = int(match.group(1))
                pin_y = int(match.group(2))
                corrected_x = height - 1 - pin_x
                pin_definition = match.group(3)
                cord = Cord(corrected_x, pin_y)

                pin_queue.put((cord, pin_definition))
                pin_dict[cord] = pin_definition
    
    return width, height, chiplet_name, pin_queue, pin_dict

def get_pin_color(pin_definition):
    for key in PIN_COLORS:
        if key in pin_definition:
            return PIN_COLORS[key]
    return PIN_COLORS["SIGNAL"]  # Default to blue for unclassified signals

def plot_chiplet(width, height, chiplet_name, pin_dict, output_path):
    fig, ax = plt.subplots(figsize=(width + 2, height + 2))
    ax.set_xlim(-250, width * 100 + 250)  # Reduce extra padding
    ax.set_ylim(-100, height * 100 + 100)
    ax.spines['top'].set_color('white')
    ax.spines['right'].set_color('white')
    ax.spines['bottom'].set_color('white')
    ax.spines['left'].set_color('white')
    
    # Draw the chip footprint
    rect = patches.Rectangle((0, 0), width * 100, height * 100, linewidth=0, edgecolor='none', facecolor=PIN_COLORS["CHIPLET"])
    ax.add_patch(rect)
    
    # Draw pins
    for x in range(height):
        for y in range(width):
            center_x = 50 + 100 * y
            center_y = 50 + 100 * x
            cord = Cord(x, y)
            
            color = get_pin_color(pin_dict.get(cord, "SIGNAL"))
            circle = patches.Circle((center_x, center_y), 20, edgecolor='black', facecolor=color)
            ax.add_patch(circle)
    
    # Add legend box in lower right corner
    legend_x = width * 100 + 25
    legend_y = len(PIN_COLORS)*40 + 50
    legend_width = 120
    legend_height = len(PIN_COLORS)*40
    # legend_box = patches.Rectangle((legend_x - 10, legend_y - 20), legend_width, legend_height, edgecolor='black', facecolor='white', linewidth=1.5)
    # ax.add_patch(legend_box)
    # ax.text(legend_x + 20, legend_y + 90, "Legend", fontsize=12, fontweight='bold')
    
    for i, (label, color) in enumerate(PIN_COLORS.items()):
        rect = patches.Rectangle((legend_x, legend_y + 30 - i * 35), 20, 20, edgecolor='black', facecolor=color)
        ax.add_patch(rect)
        ax.text(legend_x + 30, legend_y + 30 - i * 35 + 5, label, verticalalignment='center', fontsize=17)
    
    # Add title
    ax.set_title(f"{chiplet_name} {width} x {height}", fontsize=30, fontweight='bold')
    
    ax.set_xticks([])
    ax.set_yticks([])
    ax.set_aspect('equal')
    plt.savefig(output_path, bbox_inches='tight', facecolor=fig.get_facecolor())  # Remove extra padding
    plt.close()


if __name__ == '__main__':
    parser = ArgumentParser(description="Parse a chiplet file and generate a pinout diagram.")
    parser.add_argument("-i", "--input", required=True, help="Input chiplet file location")
    parser.add_argument("-o", "--output", required=True, help="Output image file location")
    parser.add_argument("--chiplet", action="store_true", help="Draw the chiplet pinout diagram")
    
    args = parser.parse_args()
    
    width, height, chiplet_name, pin_queue, pin_dict = parse_chiplet_file(args.input)
    temp_queue = queue.Queue()
    # while not pin_queue.empty():
    #     coord, definition = pin_queue.get()
    #     print(f"Location: {coord}, Definition: {definition}")
    #     temp_queue.put((coord, definition))
    print(f"Chiplet Name: {chiplet_name}")
    print(f"Chiplet Dimensions: {width}x{height}")
    
    if args.chiplet:
        plot_chiplet(width, height, chiplet_name, pin_dict, args.output)
        print(f"Pinout diagram saved to {args.output}")
    
