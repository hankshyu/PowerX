from dataclasses import dataclass
from typing import Dict
import re
import argparse
import time
from dataclasses import dataclass, field
import plotly.graph_objects as go
import plotly.colors as pc
import matplotlib.pyplot as plt
import matplotlib.pyplot as plt
import seaborn as sns
import pandas as pd
import numpy as np
from dataclasses import dataclass, field
from copy import deepcopy
from typing import List
import math
from scipy.stats import gaussian_kde
import matplotlib.ticker as ticker
import matplotlib.ticker as mticker
from matplotlib.patches import Circle
from matplotlib.collections import PatchCollection


COLORRST    = "\u001b[0m"
BLACK       = "\u001b[30m"
RED         = "\u001b[31m"
GREEN       = "\u001b[32m"
YELLOW      = "\u001b[33m"
BLUE        = "\u001b[34m"
MAGENTA     = "\u001b[35m"
CYAN        = "\u001b[36m"
WHITE       = "\u001b[37m"

PDN_LAYERS = 3

@dataclass 
class CircuitEntry:
    instance: str
    definition: str
    multiplier: float
    vdrop: float = 0.0
    current: float = 0.0
    power: float = 0.0

@dataclass
class Cord:
    x: int
    y: int
    z: int

@dataclass
class Interconnects:
    cord: Cord = field(default_factory=lambda: Cord(-1, -1, -1))
    vdrop: float = 0.0
    current: float = 0.0
    power: float = 0.0

@dataclass
class Edges:
    cord1: Cord = field(default_factory=lambda: Cord(-1, -1, -1))
    cord2: Cord = field(default_factory=lambda: Cord(-1, -1, -1))
    vdrop: float = 0.0
    current: float = 0.0
    power: float = 0.0


def parse_spice_value(val: str) -> float:
    val = val.strip().lower()
    if val == '0.' or val == '0':
        return 0.0
    units = {'t': 1e12, 'g': 1e9, 'meg': 1e6, 'k': 1e3,
             'm': 1e-3, 'u': 1e-6, 'n': 1e-9, 'p': 1e-12, 'f': 1e-15}
    for suffix, factor in units.items():
        if val.endswith(suffix):
            try:
                return float(val[:-len(suffix)]) * factor
            except ValueError:
                continue
    return float(val)

def parse_circuit_directory(lines: list[str]) -> Dict[int, CircuitEntry]:
    directory = {}
    pattern = re.compile(r"^\s*(\d+)\s+([^\s]+)\s+(\w+)\s+([\d.]+)")

    trim_prefixes = {"edge", "via", "tsv", "ubump"}

    buffer = ""
    for line in lines:
        if re.match(r"^\s*\d+\s", line):
            buffer = line.strip()
        elif buffer:
            buffer += " " + line.strip()
        else:
            continue

        match = pattern.match(buffer)
        if match:
            number = int(match.group(1))
            instance = match.group(2)
            definition = match.group(3)
            multiplier = float(match.group(4))

            if definition in trim_prefixes and instance.startswith("xeqckt."):
                instance = instance[len("xeqckt."):]
                if instance.endswith("."):
                    instance = instance[:-1]

            directory[number] = CircuitEntry(instance, definition, multiplier)
            buffer = ""

    return directory

def parse_resistor_section(lines: list[str], circuit_directory: Dict[int, CircuitEntry]):
    subckt_lines, element_lines, rvalue_lines, vdrop_lines, current_lines, power_lines = [], [], [], [], [], []

    for line in lines:
        if line.strip().startswith("subckt"):
            subckt_lines = line.split()[1:]
        elif line.strip().startswith("element"):
            element_lines = line.split()[1:]
        elif "r value" in line:
            rvalue_lines = line.split()[2:]
        elif "v drop" in line:
            vdrop_lines = line.split()[2:]
        elif "current" in line:
            current_lines = line.split()[2:]
        elif "power" in line:
            power_lines = line.split()[2:]

        # Process block
        if subckt_lines and element_lines and rvalue_lines and vdrop_lines and current_lines and power_lines:
            for idx, elem in enumerate(element_lines):
                try:
                    circuit_num_str, _ = elem.split(":")
                    circuit_num = int(circuit_num_str)
                    vdrop = parse_spice_value(vdrop_lines[idx])
                    current = parse_spice_value(current_lines[idx])
                    power = parse_spice_value(power_lines[idx])

                    if circuit_num in circuit_directory:
                        entry = circuit_directory[circuit_num]
                        entry.vdrop = abs(vdrop)
                        entry.current = abs(current)
                        entry.power = power
                except (IndexError, ValueError):
                    continue

            # Reset for next group
            subckt_lines, element_lines, rvalue_lines, vdrop_lines, current_lines, power_lines = [], [], [], [], [], []

def display_via_current_stats(vaiCurrents: list[list]):
    num_layers = len(viaCurrents)
    cols = 2
    rows = (num_layers + cols - 1) // cols  # ensures enough rows

    fig, axs = plt.subplots(rows, cols, figsize=(20, 8 * rows))
    axs = axs.flatten()

    for i, currents in enumerate(viaCurrents):
        data = np.array(currents)
        
        ax = axs[i]
        counts, bins, _ = ax.hist(data, bins=30, density=False, alpha=0.5, edgecolor='black', label='Histogram')

        # Integer ticks on Y-axis
        ax.yaxis.set_major_locator(ticker.MaxNLocator(integer=True))

        # Statistics box
        if len(data) > 0:
            median = np.median(data)
            min_val = np.min(data)
            max_val = np.max(data)
            stats_text = f"n={len(data)}\nmin={min_val:.3}\nmax={max_val:.3}\nmed={median:.3}"
            ax.text(0.97, 0.95, stats_text, transform=ax.transAxes,
                    verticalalignment='top', horizontalalignment='right',
                    bbox=dict(boxstyle='round', facecolor='white', alpha=0.7),
                    fontsize=9)

        ax.set_title(f'Via Current Histogram (Layer {i} → {i+1})')
        ax.set_xlabel('Current (A)')
        ax.set_ylabel('Count')
        ax.grid(True)
    plt.tight_layout()
    plt.show()

def display_edge_current_stats(vaiCurrents: list[list]):
    num_layers = len(edgeCurrents)
    cols = 3
    rows = (num_layers + cols - 1) // cols
    
    fig, axs = plt.subplots(rows, cols, figsize=(20, 8 * rows))
    axs = axs.flatten()

    for i, currents in enumerate(edgeCurrents):
        data = np.array(currents)
        
        ax = axs[i]
        counts, bins, _ = ax.hist(data, bins=30, density=False, alpha=0.5, edgecolor='black', label='Histogram')

        # Integer ticks on Y-axis
        ax.yaxis.set_major_locator(ticker.MaxNLocator(integer=True))

        # Statistics box
        if len(data) > 0:
            median = np.median(data)
            min_val = np.min(data)
            max_val = np.max(data)
            stats_text = f"n={len(data)}\nmin={min_val:.3}\nmax={max_val:.3}\nmed={median:.3}"
            ax.text(0.97, 0.95, stats_text, transform=ax.transAxes,
                    verticalalignment='top', horizontalalignment='right',
                    bbox=dict(boxstyle='round', facecolor='white', alpha=0.7),
                    fontsize=9)

        ax.set_title(f'Edge Current Histogram (Layer {i}')
        ax.set_xlabel('Current (A)')
        ax.set_ylabel('Count')
        ax.grid(True)

    plt.tight_layout()
    plt.show()

def display_edge_current_stats_above_avg(edgeCurrents: list[list[float]], screen_pctg = 80):
    num_layers = len(edgeCurrents)
    cols = 3
    rows = (num_layers + cols - 1) // cols

    fig, axs = plt.subplots(rows, cols, figsize=(20, 8 * rows))
    axs = axs.flatten()

    for i, currents in enumerate(edgeCurrents):
        data = np.array(currents)
        
        if len(data) == 0:
            continue

        threshold = np.percentile(data, screen_pctg)  # or 90 for even more aggressive filtering
        filtered_data = data[data > threshold]

        ax = axs[i]
        counts, bins, _ = ax.hist(filtered_data, bins=30, density=False,
                                  alpha=0.6, edgecolor='black', color='tab:red')
        bin_centers = 0.5 * (bins[:-1] + bins[1:])
        for count, x in zip(counts, bin_centers):
            if count > 0:
                ax.text(x, count + 0.01 * max(counts), str(int(count)),
                        ha='center', va='bottom', fontsize=8, rotation=90)
        
        # Set ticks every N bins
        tick_every = 5  # e.g., every 5 bins
        bin_centers = 0.5 * (bins[:-1] + bins[1:])
        tick_locs = bin_centers[::tick_every]
        ax.set_xticks(tick_locs)
        import matplotlib.ticker as mticker
        ax.xaxis.set_major_formatter(mticker.FuncFormatter(lambda x, _: f"{x:.2f}"))
        ax.yaxis.set_major_locator(ticker.MaxNLocator(integer=True))

        # Stats
        n = len(filtered_data)
        min_val = np.min(filtered_data) if n > 0 else 0
        max_val = np.max(filtered_data) if n > 0 else 0
        median_val = np.median(filtered_data) if n > 0 else 0
        stats_text = f"n={n}\nmin={min_val:.3}\nmax={max_val:.3}\nmed={median_val:.3}"
        ax.text(0.97, 0.95, stats_text, transform=ax.transAxes,
                verticalalignment='top', horizontalalignment='right',
                bbox=dict(boxstyle='round', facecolor='white', alpha=0.7),
                fontsize=9)

        ax.set_title(f'Edge Current > {screen_pctg}% (Layer {i})')
        ax.set_xlabel('Current (A)')
        ax.set_ylabel('Count')
        ax.grid(True)

    # Hide unused subplots
    for j in range(num_layers, len(axs)):
        fig.delaxes(axs[j])

    plt.tight_layout()
    plt.show()

def display_via_current_2d(viaCurrentWhole):
    num_layers = len(viaCurrentWhole)
    cols = 2
    rows = (num_layers + cols - 1) // cols

    fig, axs = plt.subplots(rows, cols, figsize=(5 * cols, 5 * rows), dpi=250)
    axs = axs.flatten()

    for i, vias in enumerate(viaCurrentWhole):
        ax = axs[i]
        if len(vias) == 0:
            ax.set_title(f"No vias on Layer {i} → {i+1}")
            ax.axis('off')
            continue

        # Extract (x, y, current)
        xs = [v.cord1.x for v in vias]
        ys = [v.cord1.y for v in vias]
        currents = [v.current for v in vias]

        # Normalize currents for radius and color
        currents_np = np.array(currents)
        norm_currents = (currents_np - np.min(currents_np)) / (np.max(currents_np) - np.min(currents_np) + 1e-12)
        radii = 0.5 + 3.5 * norm_currents  # scale radius from 0.5 to 4.0

        # Draw via as circles
        patches = []
        for x, y, r in zip(xs, ys, radii):
            circle = Circle((x, y), radius=r)
            patches.append(circle)

        p = PatchCollection(patches, cmap='Reds', alpha=0.7, edgecolor='black', linewidth=0.5)
        p.set_array(currents_np)
        ax.add_collection(p)

        ax.set_aspect('equal', 'box')
        ax.autoscale_view()
        ax.set_title(f"Via Current Map (Layer {i} → {i+1})")
        ax.set_xlabel('X')
        ax.set_ylabel('Y')
        fig.colorbar(p, ax=ax, label='Current (A)')

    # Hide unused axes
    for j in range(num_layers, len(axs)):
        fig.delaxes(axs[j])

    plt.tight_layout()
    plt.show()

def display_via_vdrop_2d(viavdropWhole):
    num_layers = len(viavdropWhole)
    cols = 2
    rows = (num_layers + cols - 1) // cols

    fig, axs = plt.subplots(rows, cols, figsize=(3.5 * cols, 3.5 * rows), dpi=150)
    axs = axs.flatten()

    for i, vias in enumerate(viavdropWhole):
        ax = axs[i]
        if len(vias) == 0:
            ax.set_title(f"Vias on Layer {i} → {i+1}")
            ax.axis('off')
            continue

        # Extract (x, y, current)
        xs = [v.cord1.x for v in vias]
        ys = [v.cord1.y for v in vias]
        vdrops = [v.vdrop for v in vias]

        # Normalize currents for radius and color
        vdrops_np = np.array(vdrops)
        norm_vdrop = (vdrops_np - np.min(vdrops_np)) / (np.max(vdrops_np) - np.min(vdrops_np) + 1e-12)
        radii = 0.4 + 3.0 * norm_vdrop  # scale radius from 0.5 to 4.0

        # Draw via as circles
        patches = []
        for x, y, r in zip(xs, ys, radii):
            circle = Circle((x, y), radius=r)
            patches.append(circle)

        p = PatchCollection(patches, cmap='Reds', alpha=0.7, edgecolor='black', linewidth=0.5)
        p.set_array(vdrops_np)
        ax.add_collection(p)

        ax.set_aspect('equal', 'box')
        ax.autoscale_view()
        ax.set_title(f"Via Vdrop Map (Layer {i} → {i+1})")
        ax.set_xlabel('X')
        ax.set_ylabel('Y')
        fig.colorbar(p, ax=ax, fraction=0.046, pad=0.04, label='Voltage (V)')

    # Hide unused axes
    for j in range(num_layers, len(axs)):
        fig.delaxes(axs[j])

    plt.tight_layout()
    plt.show()

def display_via_vdrop_stats(viaVdrop: list[list[float]]):
    num_layers = len(viaVdrop)
    cols = 2
    rows = (num_layers + cols - 1) // cols  # ensures enough rows

    fig, axs = plt.subplots(rows, cols, figsize=(15, 8 * rows))
    axs = axs.flatten()

    for i, currents in enumerate(viaVdrop):
        data = np.array(currents)
        
        ax = axs[i]
        counts, bins, _ = ax.hist(data, bins=30, density=False, alpha=0.5, edgecolor='black', label='Histogram')

        # Integer ticks on Y-axis
        ax.yaxis.set_major_locator(ticker.MaxNLocator(integer=True))

        # Statistics box
        if len(data) > 0:
            median = np.median(data)
            min_val = np.min(data)
            max_val = np.max(data)
            stats_text = f"n={len(data)}\nmin={min_val:.3}\nmax={max_val:.3}\nmed={median:.3}"
            ax.text(0.97, 0.95, stats_text, transform=ax.transAxes,
                    verticalalignment='top', horizontalalignment='right',
                    bbox=dict(boxstyle='round', facecolor='white', alpha=0.7),
                    fontsize=7)

        ax.set_title(f'Via V Drop Histogram (Layer {i} → {i+1})')
        ax.set_xlabel('Voltage (V)')
        ax.set_ylabel('Count')
        ax.grid(True)
    plt.tight_layout()
    plt.show()

def display_edge_vdrop_stats(edgeVdrop: list[list]):
    num_layers = len(edgeVdrop)
    cols = 3
    rows = (num_layers + cols - 1) // cols
    
    fig, axs = plt.subplots(rows, cols, figsize=(20, 8 * rows))
    axs = axs.flatten()

    for i, vdrop in enumerate(edgeVdrop):
        data = np.array(vdrop)
        
        ax = axs[i]
        counts, bins, _ = ax.hist(data, bins=30, density=False, alpha=0.5, edgecolor='black', label='Histogram')

        # Integer ticks on Y-axis
        ax.yaxis.set_major_locator(ticker.MaxNLocator(integer=True))

        # Statistics box
        if len(data) > 0:
            median = np.median(data)
            min_val = np.min(data)
            max_val = np.max(data)
            stats_text = f"n={len(data)}\nmin={min_val:.3}\nmax={max_val:.3}\nmed={median:.3}"
            ax.text(0.97, 0.95, stats_text, transform=ax.transAxes,
                    verticalalignment='top', horizontalalignment='right',
                    bbox=dict(boxstyle='round', facecolor='white', alpha=0.7),
                    fontsize=9)

        ax.set_title(f'Edge V drop Histogram (Layer {i})')
        ax.set_xlabel('Voltage (V)')
        ax.set_ylabel('Count')
        ax.grid(True)

    plt.tight_layout()
    plt.show()

def display_edge_vdrop_stats_above_avg(edgeVdrop: list[list[float]], screen_pctg = 80):
    num_layers = len(edgeVdrop)
    cols = 3
    rows = (num_layers + cols - 1) // cols

    fig, axs = plt.subplots(rows, cols, figsize=(20, 8 * rows))
    axs = axs.flatten()

    for i, currents in enumerate(edgeVdrop):
        data = np.array(currents)
        
        if len(data) == 0:
            continue

        threshold = np.percentile(data, screen_pctg)  # or 90 for even more aggressive filtering
        filtered_data = data[data > threshold]

        ax = axs[i]
        counts, bins, _ = ax.hist(filtered_data, bins=30, density=False,
                                  alpha=0.6, edgecolor='black', color='tab:red')
        bin_centers = 0.5 * (bins[:-1] + bins[1:])
        for count, x in zip(counts, bin_centers):
            if count > 0:
                ax.text(x, count + 0.01 * max(counts), str(int(count)),
                        ha='center', va='bottom', fontsize=8, rotation=90)
        
        # Set ticks every N bins
        tick_every = 5  # e.g., every 5 bins
        bin_centers = 0.5 * (bins[:-1] + bins[1:])
        tick_locs = bin_centers[::tick_every]
        ax.set_xticks(tick_locs)
        import matplotlib.ticker as mticker
        ax.xaxis.set_major_formatter(mticker.FuncFormatter(lambda x, _: f"{x:.2f}"))
        ax.yaxis.set_major_locator(ticker.MaxNLocator(integer=True))

        # Stats
        n = len(filtered_data)
        min_val = np.min(filtered_data) if n > 0 else 0
        max_val = np.max(filtered_data) if n > 0 else 0
        median_val = np.median(filtered_data) if n > 0 else 0
        stats_text = f"n={n}\nmin={min_val:.3}\nmax={max_val:.3}\nmed={median_val:.3}"
        ax.text(0.97, 0.95, stats_text, transform=ax.transAxes,
                verticalalignment='top', horizontalalignment='right',
                bbox=dict(boxstyle='round', facecolor='white', alpha=0.7),
                fontsize=9)

        ax.set_title(f'Edge Vdrop > {screen_pctg}% (Layer {i})')
        ax.set_xlabel('Voltage (V)')
        ax.set_ylabel('Count')
        ax.grid(True)

    # Hide unused subplots
    for j in range(num_layers, len(axs)):
        fig.delaxes(axs[j])

    plt.tight_layout()
    plt.show()

def display_via_power_2d(viaPowerWhole):
    num_layers = len(viaPowerWhole)
    cols = 2
    rows = (num_layers + cols - 1) // cols

    fig, axs = plt.subplots(rows, cols, figsize=(3.5 * cols, 3.5 * rows), dpi=150)
    axs = axs.flatten()

    for i, vias in enumerate(viaPowerWhole):
        ax = axs[i]
        if len(vias) == 0:
            ax.set_title(f"Vias on Layer {i} → {i+1}")
            ax.axis('off')
            continue

        # Extract (x, y, current)
        xs = [v.cord1.x for v in vias]
        ys = [v.cord1.y for v in vias]
        powers = [v.power for v in vias]

        # Normalize currents for radius and color
        power_np = np.array(powers)
        norm_power = (power_np - np.min(power_np)) / (np.max(power_np) - np.min(power_np) + 1e-12)
        radii = 0.4 + 3.0 * norm_power  # scale radius from 0.5 to 4.0

        # Draw via as circles
        patches = []
        for x, y, r in zip(xs, ys, radii):
            circle = Circle((x, y), radius=r)
            patches.append(circle)

        p = PatchCollection(patches, cmap='Reds', alpha=0.7, edgecolor='black', linewidth=0.5)
        p.set_array(power_np)
        ax.add_collection(p)

        ax.set_aspect('equal', 'box')
        ax.autoscale_view()
        ax.set_title(f"Via Power Map (Layer {i} → {i+1})")
        ax.set_xlabel('X')
        ax.set_ylabel('Y')
        fig.colorbar(p, ax=ax, fraction=0.046, pad=0.04, label='Watt (J/s)')

    # Hide unused axes
    for j in range(num_layers, len(axs)):
        fig.delaxes(axs[j])

    plt.tight_layout()
    plt.show()

def display_via_power_stats(viaPower: list[list[float]]):
    num_layers = len(viaPower)
    cols = 2
    rows = (num_layers + cols - 1) // cols  # ensures enough rows

    fig, axs = plt.subplots(rows, cols, figsize=(15, 8 * rows))
    axs = axs.flatten()

    for i, power in enumerate(viaPower):
        data = np.array(power)
        ax = axs[i]

        # Histogram (counts + bins)
        counts, bins, _ = ax.hist(data, bins=30, density=False, alpha=0.5, edgecolor='black')

        # Integer ticks on Y-axis
        ax.yaxis.set_major_locator(ticker.MaxNLocator(integer=True))

        # Calculate stats
        if len(data) > 0:
            median = np.median(data)
            min_val = np.min(data)
            max_val = np.max(data)
            total = np.sum(data)

            # Statistics box
            stats_text = (
                f"n={len(data)}\n"
                f"min={min_val:.3e}\n"
                f"max={max_val:.3e}\n"
                f"med={median:.3e}\n"
                f"sum={total:.3e}"
            )
            ax.text(0.97, 0.95, stats_text, transform=ax.transAxes,
                    verticalalignment='top', horizontalalignment='right',
                    bbox=dict(boxstyle='round', facecolor='white', alpha=0.7),
                    fontsize=7)

            # Add cumulative percentage curve
            hist_vals, bin_edges = np.histogram(data, bins=30)
            cumsum = np.cumsum(hist_vals)
            cdf = cumsum / cumsum[-1]  # Normalize to 1.0
            bin_centers = 0.5 * (bin_edges[:-1] + bin_edges[1:])

            ax2 = ax.twinx()
            ax2.plot(bin_centers, cdf * 100, color='tab:red', linestyle='--', linewidth=1.5, label='Cumulative %')
            ax2.set_ylabel('Cumulative %')
            ax2.set_ylim(0, 100)
            ax2.tick_params(axis='y', colors='tab:red')
            ax2.yaxis.label.set_color('tab:red')
            ax2.yaxis.set_major_locator(ticker.MultipleLocator(20))
            ax2.grid(False)

        ax.set_title(f'Via Power Histogram (Layer {i} → {i+1})')
        ax.set_xlabel('Watt (J/s)')
        ax.set_ylabel('Count')
        ax.grid(True)
    
    plt.tight_layout()
    plt.show()

def display_edge_power_stats(edgePower: list[list]):
    num_layers = len(edgePower)
    cols = 3
    rows = (num_layers + cols - 1) // cols
    
    fig, axs = plt.subplots(rows, cols, figsize=(20, 8 * rows))
    axs = axs.flatten()

    for i, power in enumerate(edgePower):
        data = np.array(power)
        
        ax = axs[i]
        counts, bins, _ = ax.hist(data, bins=30, density=False, alpha=0.5, edgecolor='black', label='Histogram')

        # Integer ticks on Y-axis
        ax.yaxis.set_major_locator(ticker.MaxNLocator(integer=True))

        # Statistics box
        if len(data) > 0:
            median = np.median(data)
            min_val = np.min(data)
            max_val = np.max(data)
            total = np.sum(data)
            # Statistics box
            stats_text = (
                f"n={len(data)}\n"
                f"min={min_val:.3e}\n"
                f"max={max_val:.3e}\n"
                f"med={median:.3e}\n"
                f"sum={total:.3e}"
            )
            ax.text(0.97, 0.95, stats_text, transform=ax.transAxes,
                    verticalalignment='top', horizontalalignment='right',
                    bbox=dict(boxstyle='round', facecolor='white', alpha=0.7),
                    fontsize=9)

        ax.set_title(f'Edge Power Histogram (Layer {i})')
        ax.set_xlabel('Watt (J/s)')
        ax.set_ylabel('Count')
        ax.grid(True)

    plt.tight_layout()
    plt.show()

def display_edge_power_stats_above_avg(edgePower: list[list[float]], screen_pctg = 80):
    num_layers = len(edgePower)
    cols = 3
    rows = (num_layers + cols - 1) // cols

    fig, axs = plt.subplots(rows, cols, figsize=(15, 8 * rows))
    axs = axs.flatten()

    for i, currents in enumerate(edgePower):
        data = np.array(currents)
        
        if len(data) == 0:
            continue

        threshold = np.percentile(data, screen_pctg)  # or 90 for even more aggressive filtering
        filtered_data = data[data > threshold]

        ax = axs[i]
        counts, bins, _ = ax.hist(filtered_data, bins=30, density=False,
                                  alpha=0.6, edgecolor='black', color='tab:red')
        bin_centers = 0.5 * (bins[:-1] + bins[1:])
        for count, x in zip(counts, bin_centers):
            if count > 0:
                ax.text(x, count + 0.01 * max(counts), str(int(count)),
                        ha='center', va='bottom', fontsize=8, rotation=90)
        
        # Set ticks every N bins
        tick_every = 5  # e.g., every 5 bins
        bin_centers = 0.5 * (bins[:-1] + bins[1:])
        tick_locs = bin_centers[::tick_every]
        ax.set_xticks(tick_locs)
        import matplotlib.ticker as mticker
        ax.xaxis.set_major_formatter(mticker.FuncFormatter(lambda x, _: f"{x:.2f}"))
        ax.yaxis.set_major_locator(ticker.MaxNLocator(integer=True))

        # Stats
        n = len(filtered_data)
        min_val = np.min(filtered_data) if n > 0 else 0
        max_val = np.max(filtered_data) if n > 0 else 0
        median_val = np.median(filtered_data) if n > 0 else 0
        stats_text = f"n={n}\nmin={min_val:.3}\nmax={max_val:.3}\nmed={median_val:.3}"
        
        ax.text(0.97, 0.95, stats_text, transform=ax.transAxes,
                verticalalignment='top', horizontalalignment='right',
                bbox=dict(boxstyle='round', facecolor='white', alpha=0.7),
                fontsize=9)

        ax.set_title(f'Edge Power > {screen_pctg}% (Layer {i})')
        ax.set_xlabel('Watt (J/s)')
        ax.set_ylabel('Count')
        ax.grid(True)

    # Hide unused subplots
    for j in range(num_layers, len(axs)):
        fig.delaxes(axs[j])

    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Parse HSPICE .lis file and visualize resistor metrics")
    parser.add_argument("-i", "--input", required=True, help="Path to the .lis file")
    parser.add_argument("-c", "--currStats", action="store_true", help="currrent distribution related statistics")
    parser.add_argument("-d", "--vdropStats", action="store_true", help="IR drop related statistics")
    parser.add_argument("-e", "--powerStats", action="store_true", help="power related statistics")
    parser.add_argument("-V", "--vdrop", action="store_true", help="Visualize voltage drop")
    parser.add_argument("-I", "--current", action="store_true", help="Visualize current")
    parser.add_argument("-P", "--power", action="store_true", help="Visualize power")


    parser.add_argument("-v", "--verbose", action="store_true", help="Enable verbose mode")
    args = parser.parse_args()

    if args.verbose:
        selected_mode = (
            "Vdrop" if args.vdrop else
            "Current" if args.current else
            "Power" if args.power else
            "Distribution"
        )
        print(CYAN,"IRISLAB Power Plane Rendering Program ", COLORRST)
        print(f"[INFO] Time: {time.strftime('%Y-%m-%d %H:%M:%S')}")
        print(f"[INFO] Input file: {args.input}")
        print(f"[INFO] Mode selected: {selected_mode}")

    try:
        with open(args.input) as f:
            lines = f.readlines()

        # Parse circuit name directory
        start = next(i for i, line in enumerate(lines) if "circuit name directory" in line)
        end = next(i for i in range(start + 1, len(lines)) if lines[i].strip() == "")
        directory_section = lines[start:end]
        circuit_directory = parse_circuit_directory(directory_section)

        # Parse resistor report (guaranteed to come after)
        resistor_start = next(i for i in range(end, len(lines)) if "**** resistors" in lines[i])
        resistor_lines = lines[resistor_start:]
        parse_resistor_section(resistor_lines, circuit_directory)

        # Show example output
        print("Total circuit entries parsed:", len(circuit_directory))

        minVdrop = 99999999
        maxVdrop = -1
        minCurrent = 99999999
        maxCurrent = -1
        totalPower = 0
        minPower = 99999999
        maxPower = -1

        tsvArrRaw = []
        ubumpArrRaw = []
        edgeArrRaw = []
        viaArrRaw = []

        maxLayer = 0
        
        render_components = {"edge", "via", "tsv", "ubump"}

        for num, entry in circuit_directory.items():
                if entry.current == 0: continue
                if entry.definition in render_components:
                    if entry.vdrop < minVdrop: minVdrop = entry.vdrop
                    if entry.vdrop > maxVdrop: maxVdrop = entry.vdrop
                    if entry.current < minCurrent: minCurrent = entry.current
                    if entry.current > maxCurrent: maxCurrent = entry.current
                    if entry.power < minPower: minPower = entry.power
                    if entry.power > maxPower: maxPower = entry.power
                    totalPower += entry.power

                    if entry.definition == "ubump":
                        ub = Interconnects()
                        match = re.search(r"n(\d+)_(\d+)_(\d+)", entry.instance)
                        if match:
                            x, y, z = map(int, match.groups())
                            if z > maxLayer: maxLayer = z
                            ub.cord = Cord(x, y, z)
                        else:
                            print(f"[Error parsing] {entry.instance}")
                            exit()
                        ub.vdrop = entry.vdrop
                        ub.current = entry.current
                        ub.power = entry.power
                        ubumpArrRaw.append(ub)
                    elif entry.definition == "tsv":
                        tsv = Interconnects()
                        match = re.search(r"n(\d+)_(\d+)_(\d+)", entry.instance)
                        if match:
                            x, y, z = map(int, match.groups())
                            if z > maxLayer: maxLayer = z
                            tsv.cord = Cord(x, y, z)
                        else:
                            print(f"[Error parsing] {entry.instance}")
                            exit()
                        tsv.vdrop = entry.vdrop
                        tsv.current = entry.current
                        tsv.power = entry.power
                        tsvArrRaw.append(tsv)
                    elif entry.definition == "edge":
                        eg = Edges()
                        matches = re.findall(r"n(\d+)_(\d+)_(\d+)", entry.instance)
                        if len(matches) == 2:
                            eg.cord1 = Cord(*map(int, matches[0]))
                            eg.cord2 = Cord(*map(int, matches[1]))
                            if eg.cord1.z > maxLayer: maxLayer = eg.cord1.z
                            if eg.cord2.z > maxLayer: maxLayer = eg.cord2.z
                        else:
                            print(f"[Error parsing] {entry.instance}")
                            exit()
                        eg.vdrop = entry.vdrop
                        eg.current = entry.current
                        eg.power = entry.power
                        edgeArrRaw.append(eg)
                    elif entry.definition == "via":
                        va = Edges()
                        matches = re.findall(r"n(\d+)_(\d+)_(\d+)", entry.instance)
                        if len(matches) == 2:
                            va.cord1 = Cord(*map(int, matches[0]))
                            va.cord2 = Cord(*map(int, matches[1]))
                            if va.cord1.z > maxLayer: maxLayer = va.cord1.z
                            if va.cord2.z > maxLayer: maxLayer = va.cord2.z
                        else:
                            print(f"[Error parsing] {entry.instance}")
                            exit()
                        va.vdrop = entry.vdrop
                        va.current = entry.current
                        va.power = entry.power
                        viaArrRaw.append(va)

        ubumpArr = []
        edgeArr = []
        viaArr = []
        tsvArr = []
        # conduct transformaton to the coordiantes
        for entry in edgeArrRaw:
            eg = deepcopy(entry)
            eg.cord1.z = maxLayer - eg.cord1.z
            eg.cord2.z = maxLayer - eg.cord2.z
            edgeArr.append(eg)
        
        for entry in viaArrRaw:
            va = deepcopy(entry)
            va.cord1.z = maxLayer - va.cord1.z
            va.cord2.z = maxLayer - va.cord2.z
            viaArr.append(va)

        for entry in tsvArrRaw:
            tv = Interconnects()
            tv.cord1 = entry.cord
            tv.cord1.z = maxLayer - tv.cord1.z
            tv.cord2 = deepcopy(tv.cord1)
            tv.cord2.z = -0.4
            tv.vdrop = entry.vdrop
            tv.current = entry.current
            tv.power = entry.power
            tsvArr.append(tv)

        for entry in ubumpArrRaw:
            ub = Interconnects()
            ub.cord1 = entry.cord
            ub.cord1.z = maxLayer - ub.cord1.z
            ub.cord2 = deepcopy(ub.cord1)
            ub.cord2.z = maxLayer + 0.4
            ub.vdrop = entry.vdrop
            ub.current = entry.current
            ub.power = entry.power
            ubumpArr.append(ub)
            
        print(f"Final Min/Max Vdrop:{minVdrop}/{maxVdrop}, Current:{minCurrent}/{maxCurrent}, Total Power:{totalPower}, Max Power:{maxPower}")
        if args.currStats:
            
            viaCurrents = []
            viaCurrentWhole = []
            for i in range(maxLayer):
                viaCurrents.append([])
                viaCurrentWhole.append([])

            for elem in viaArr:
                for i in range(len(viaCurrents)):
                    if((elem.cord1.z == i) or (elem.cord2.z == i)) and ((elem.cord1.z == (i+1)) or (elem.cord2.z == (i+1))):
                        viaCurrents[i].append(elem.current)
                        viaCurrentWhole[i].append(elem)
            
            display_via_current_2d(viaCurrentWhole)
            display_via_current_stats(viaCurrents)
            


            edgeCurrents = []
            for i in range(maxLayer + 1):
                edgeCurrents.append([])

            for elem in edgeArr:
                edgeCurrents[elem.cord1.z].append(elem.current)
            
            display_edge_current_stats(edgeCurrents)
            display_edge_current_stats_above_avg(edgeCurrents)
        elif args.vdropStats:
            viaVdrop = []
            viavdropWhole = []
            for i in range(maxLayer):
                viaVdrop.append([])
                viavdropWhole.append([])

            for elem in viaArr:
                for i in range(len(viaVdrop)):
                    if((elem.cord1.z == i) or (elem.cord2.z == i)) and ((elem.cord1.z == (i+1)) or (elem.cord2.z == (i+1))):
                        viaVdrop[i].append(elem.vdrop)
                        viavdropWhole[i].append(elem)
            
            display_via_vdrop_2d(viavdropWhole)
            display_via_vdrop_stats(viaVdrop)
            


            edgeVdrop = []
            for i in range(maxLayer + 1):
                edgeVdrop.append([])

            for elem in edgeArr:
                edgeVdrop[elem.cord1.z].append(elem.vdrop)
            
            display_edge_vdrop_stats(edgeVdrop)
            display_edge_vdrop_stats_above_avg(edgeVdrop)
        elif args.powerStats:
            viaPower = []
            viaPowerWhole = []
            for i in range(maxLayer):
                viaPower.append([])
                viaPowerWhole.append([])

            for elem in viaArr:
                for i in range(len(viaPower)):
                    if((elem.cord1.z == i) or (elem.cord2.z == i)) and ((elem.cord1.z == (i+1)) or (elem.cord2.z == (i+1))):
                        viaPower[i].append(elem.power)
                        viaPowerWhole[i].append(elem)
            
            display_via_power_2d(viaPowerWhole)
            display_via_power_stats(viaPower)
            
            edgePower = []
            for i in range(maxLayer + 1):
                edgePower.append([])

            for elem in edgeArr:
                edgePower[elem.cord1.z].append(elem.power)
            
            display_edge_power_stats(edgePower)
            display_edge_power_stats_above_avg(edgePower)
            

        if args.vdrop:
            fig = go.Figure()
            colorscale = pc.sequential.Turbo  # Replaced RdBu with Turbo for smooth gradient

            # Small epsilon to avoid log(0)
            epsilon = 1e-6

            # Compute log-scale min/max safely
            log_minV = math.log10(minVdrop + epsilon)
            log_maxV = math.log10(maxVdrop + epsilon)

            # Logarithmic color mapper
            def vdrop_to_color_log(v):
                norm_log_v = (math.log10(v + epsilon) - log_minV) / (log_maxV - log_minV)
                return pc.sample_colorscale(colorscale, norm_log_v)[0]

            # Render all edges with log-colored vdrop
            # for arr in [edgeArr, viaArr, ubumpArr, tsvArr]:  # or add more: ubumpArr, tsvArr
            for arr in [edgeArr, viaArr]:  # or add more: ubumpArr, tsvArr
                for edge in arr:
                    fig.add_trace(go.Scatter3d(
                        x=[edge.cord1.x, edge.cord2.x],
                        y=[edge.cord1.y, edge.cord2.y],
                        z=[edge.cord1.z, edge.cord2.z],
                        mode='lines',
                        line=dict(color=vdrop_to_color_log(edge.vdrop), width=3),
                        hoverinfo='skip',
                        showlegend=False
                    ))

            # Colorbar with log-scale ticks mapped to actual voltages
            colorbar_trace = go.Scatter3d(
                x=[None], y=[None], z=[None],
                mode='markers',
                marker=dict(
                    colorscale=colorscale,
                    cmin=log_minV,
                    cmax=log_maxV,
                    color=[log_minV, log_maxV],
                    size=0,
                    showscale=True,
                    colorbar=dict(
                        title='VDrop (V)',
                        thickness=20,
                        tickvals=[
                            log_minV,
                            (log_minV + log_maxV) / 2,
                            log_maxV
                        ],
                        ticktext=[
                            f"{10**log_minV:.4f}",
                            f"{10**((log_minV + log_maxV)/2):.4f}",
                            f"{10**log_maxV:.4f}"
                        ]
                    )
                ),
                showlegend=False
            )
            fig.add_trace(colorbar_trace)

            fig.update_layout(scene=dict(
                xaxis_title='X',
                yaxis_title='Y',
                zaxis_title='Z'
            ))

            fig.write_html("myplot.html")
            import subprocess
            subprocess.run(["open", "-a", "Google Chrome", "myplot.html"])

        elif args.current:
            fig = go.Figure()
            colorscale = pc.sequential.Turbo  # Replaced RdBu with Turbo for smooth gradient

            # Small epsilon to avoid log(0)
            epsilon = 1e-6

            # Compute log-scale min/max safely
            log_minV = math.log10(minCurrent + epsilon)
            log_maxV = math.log10(maxCurrent + epsilon)

            # Logarithmic color mapper
            def current_to_color_log(c):
                norm_log_v = (math.log10(c + epsilon) - log_minV) / (log_maxV - log_minV)
                return pc.sample_colorscale(colorscale, norm_log_v)[0]

            # Render all edges with log-colored vdrop
            # for arr in [edgeArr, viaArr, ubumpArr, tsvArr]:  # or add more: ubumpArr, tsvArr
            for arr in [edgeArr, viaArr]:  # or add more: ubumpArr, tsvArr
                for edge in arr:
                    fig.add_trace(go.Scatter3d(
                        x=[edge.cord1.x, edge.cord2.x],
                        y=[edge.cord1.y, edge.cord2.y],
                        z=[edge.cord1.z, edge.cord2.z],
                        mode='lines',
                        line=dict(color=current_to_color_log(edge.current), width=3),
                        hoverinfo='skip',
                        showlegend=False
                    ))

            # Colorbar with log-scale ticks mapped to actual voltages
            colorbar_trace = go.Scatter3d(
                x=[None], y=[None], z=[None],
                mode='markers',
                marker=dict(
                    colorscale=colorscale,
                    cmin=log_minV,
                    cmax=log_maxV,
                    color=[log_minV, log_maxV],
                    size=0,
                    showscale=True,
                    colorbar=dict(
                        title='Current (I)',
                        thickness=20,
                        tickvals=[
                            log_minV,
                            (log_minV + log_maxV) / 2,
                            log_maxV
                        ],
                        ticktext=[
                            f"{10**log_minV:.4f}",
                            f"{10**((log_minV + log_maxV)/2):.4f}",
                            f"{10**log_maxV:.4f}"
                        ]
                    )
                ),
                showlegend=False
            )
            fig.add_trace(colorbar_trace)

            fig.update_layout(scene=dict(
                xaxis_title='X',
                yaxis_title='Y',
                zaxis_title='Z'
            ))

            fig.write_html("myplot.html")
            import subprocess
            subprocess.run(["open", "-a", "Google Chrome", "myplot.html"])
        elif args.power:
            fig = go.Figure()
            colorscale = pc.sequential.Turbo  # Replaced RdBu with Turbo for smooth gradient

            # Small epsilon to avoid log(0)
            epsilon = 1e-6

            # Compute log-scale min/max safely
            log_minP = math.log10(minPower + epsilon)
            log_maxP = math.log10(maxPower + epsilon)

            # Logarithmic color mapper
            def power_to_color_log(v):
                norm_log_p = (math.log10(v + epsilon) - log_minP) / (log_maxP - log_minP)
                return pc.sample_colorscale(colorscale, norm_log_p)[0]

            # Render all edges with log-colored vdrop
            # for arr in [edgeArr, viaArr, ubumpArr, tsvArr]:  # or add more: ubumpArr, tsvArr
            for arr in [edgeArr, viaArr]:  # or add more: ubumpArr, tsvArr
                for edge in arr:
                    fig.add_trace(go.Scatter3d(
                        x=[edge.cord1.x, edge.cord2.x],
                        y=[edge.cord1.y, edge.cord2.y],
                        z=[edge.cord1.z, edge.cord2.z],
                        mode='lines',
                        line=dict(color=power_to_color_log(edge.power), width=3),
                        hoverinfo='skip',
                        showlegend=False
                    ))

            # Colorbar with log-scale ticks mapped to actual voltages
            colorbar_trace = go.Scatter3d(
                x=[None], y=[None], z=[None],
                mode='markers',
                marker=dict(
                    colorscale=colorscale,
                    cmin=log_minP,
                    cmax=log_maxP,
                    color=[log_minP, log_maxP],
                    size=0,
                    showscale=True,
                    colorbar=dict(
                        title='Power (W)',
                        thickness=20,
                        tickvals=[
                            log_minP,
                            (log_minP + log_maxP) / 2,
                            log_maxP
                        ],
                        ticktext=[
                            f"{10**log_minP:.3e}",
                            f"{10**((log_minP + log_maxP)/2):.3e}",
                            f"{10**log_maxP:.3e}"
                        ]
                    )
                ),
                showlegend=False
            )
            fig.add_trace(colorbar_trace)

            fig.update_layout(scene=dict(
                xaxis_title='X',
                yaxis_title='Y',
                zaxis_title='Z'
            ))

            fig.write_html("myplot.html")
            import subprocess
            subprocess.run(["open", "-a", "Google Chrome", "myplot.html"])
    
    except FileNotFoundError:
        print(f"Error: File not found at {args.input}")
    except StopIteration:
        print("Error: Could not locate required sections in the file.")