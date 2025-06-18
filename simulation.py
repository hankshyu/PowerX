import numpy as np
import matplotlib.pyplot as plt

def sample_uniform_in_circle(radius=10, num_points=10000):
    """Sample uniformly within a circle using polar coordinates."""
    theta = np.random.uniform(0, 2 * np.pi, num_points)
    r = radius * np.sqrt(np.random.uniform(0, 1, num_points))
    x = r * np.cos(theta)
    y = r * np.sin(theta)
    return np.stack((x, y), axis=-1)


def sample_uniform_in_cube(edge_length=10, num_points=10000, dimension=2):
    """
    Uniformly sample points in a d-dimensional cube centered at the origin.
    
    Parameters:
        edge_length (float): Length of each cube side.
        num_points (int): Number of points to sample.
        dimension (int): Number of dimensions (2 for square, 3 for cube, etc.).
    
    Returns:
        np.ndarray: Array of shape (num_points, dimension) containing sampled points.
    """
    half = edge_length / 2
    return np.random.uniform(-half, half, size=(num_points, dimension))

def main():
    # Step 1: Sample points
    # points = sample_uniform_in_circle(radius=10, num_points=10000)
    points = sample_uniform_in_cube(edge_length=30, num_points=100000000, dimension=2)
    # Step 2: Plot the sampled points
    plt.figure(figsize=(6, 6))
    plt.scatter(points[:, 0], points[:, 1], s=1, alpha=0.5)
    plt.gca().set_aspect('equal')
    plt.title("Uniform Sampling Inside a Circle (r = 10)")
    plt.grid(True)
    # plt.show()

    # Step 3: Compute L1 and L2 distances from the origin
    l1_distances = np.sum(np.abs(points), axis=1)
    l2_distances = np.linalg.norm(points, axis=1)

    # Step 4: Avoid division by zero and compute L1/L2
    mask = l2_distances > 0
    ratios = l1_distances[mask] / l2_distances[mask]

    # Step 5: Compute confidence quantiles
    q90 = np.quantile(ratios, 0.10)
    q95 = np.quantile(ratios, 0.05)
    q99 = np.quantile(ratios, 0.01)

    # Step 6: Plot histogram with vertical confidence lines and labels
    plt.figure(figsize=(10, 6))
    plt.hist(ratios, bins=100, density=True, color='orange', edgecolor='black', alpha=0.7)
    plt.axvline(q90, color='blue', linestyle='--', label=f"90% ≤ {q90:.9f}")
    plt.axvline(q95, color='green', linestyle='--', label=f"95% ≤ {q95:.9f}")
    plt.axvline(q99, color='red', linestyle='--', label=f"99% ≤ {q99:.9f}")

    # Add value labels next to each line
    plt.text(q90 + 0.005, plt.ylim()[1] * 0.9, f"{q90:.3f}", color='blue', rotation=90, va='top')
    plt.text(q95 + 0.005, plt.ylim()[1] * 0.6, f"{q95:.3f}", color='green', rotation=90, va='top')
    plt.text(q99 + 0.005, plt.ylim()[1] * 0.3, f"{q99:.3f}", color='red', rotation=90, va='top')

    plt.title("Distribution of L1 / L2 for Points Sampled in a Circle")
    plt.xlabel("L1 / L2 Ratio")
    plt.ylabel("Density")
    plt.grid(True)
    plt.legend()
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    main()