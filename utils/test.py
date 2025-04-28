import matplotlib.pyplot as plt
from matplotlib.path import Path
import matplotlib.patches as patches
import numpy as np

import matplotlib.pyplot as plt
from matplotlib.path import Path
import matplotlib.patches as patches

def signed_area(x, y):
    return 0.5 * sum(x[i] * y[(i + 1) % len(x)] - x[(i + 1) % len(x)] * y[i] for i in range(len(x)))

def correct_winding(x, y, desired_ccw=True):
    area = signed_area(x, y)
    if (area > 0) != desired_ccw:  # mismatch
        return x[::-1], y[::-1]
    return x, y

def append_path(x, y):
    points = list(zip(x, y))
    verts = [(pt[0], pt[1]) for pt in points]
    codes = [Path.MOVETO] + [Path.LINETO] * (len(points) - 2) + [Path.CLOSEPOLY]
    return verts, codes

# Example polygon
pieceWindingX = [0, 10, 10, 0, 0]
pieceWindingY = [0, 0, 10, 10, 0]

holeWindingArrX = [
    [2, 4, 4, 2, 2],
    [6, 8, 8, 6, 6]
]
holeWindingArrY = [
    [2, 2, 4, 4, 2],
    [6, 6, 8, 8, 6]
]

# Ensure correct winding
pieceWindingX, pieceWindingY = correct_winding(pieceWindingX, pieceWindingY, desired_ccw=True)

vertices = []
codes = []

ext_verts, ext_codes = append_path(pieceWindingX, pieceWindingY)
vertices.extend(ext_verts)
codes.extend(ext_codes)

# Fix winding for holes to be CW
for hx, hy in zip(holeWindingArrX, holeWindingArrY):
    hx, hy = correct_winding(hx, hy, desired_ccw=False)
    hole_verts, hole_codes = append_path(hx, hy)
    vertices.extend(hole_verts)
    codes.extend(hole_codes)

# Create PathPatch
path = Path(vertices, codes)
patch = patches.PathPatch(path, facecolor='orange', edgecolor='black', lw=2)

# Plot
fig, ax = plt.subplots()
ax.add_patch(patch)

# Underlay test
ax.plot([0, 10], [0, 10], 'b--')

ax.set_xlim(-1, 11)
ax.set_ylim(-1, 11)
ax.set_aspect('equal')
plt.show()