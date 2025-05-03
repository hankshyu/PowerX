import csv
import random
import argparse

def excel_style_label(n):
    """Convert number to Excel-style label: 1 -> A, 27 -> AA, etc."""
    label = ''
    while n > 0:
        n, r = divmod(n - 1, 26)
        label = chr(65 + r) + label
    return label

def generate_grid(output_file, size):
    rows = [excel_style_label(i) for i in range(1, size + 1)]
    cols = [f"{i:02d}" for i in range(1, size + 1)]

    with open(output_file, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(['GridID', 'Value'])

        for row in rows:
            for col in cols:
                grid_id = f"{row}{col}"
                value = "OBSTACLE" if random.random() < 0.1 else "EMPTY"
                writer.writerow([grid_id, value])

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Generate CSV grid with OBSTACLE/EMPTY values.')
    parser.add_argument('-o', '--output', type=str, required=True, help='Output CSV filename')
    parser.add_argument('--size', type=int, default=1000, help='Grid size (default: 1000x1000)')
    args = parser.parse_args()

    generate_grid(args.output, args.size)