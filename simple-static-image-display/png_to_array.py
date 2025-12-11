from PIL import Image
import sys

def png_to_c_array_with_coords(png_path, array_name="frame"):
    # Load image
    img = Image.open(png_path).convert("RGB")

    # Ensure size is exactly 80x80
    if img.size != (80, 80):
        raise ValueError(f"Image must be 80x80, but is {img.size}")

    pixels = img.load()

    print(f"const uint8_t {array_name}[80][80][3] PROGMEM = {{")

    for y in range(80):
        print(f"  // -------- Row {y} --------")
        print("  {")
        for x in range(80):
            r, g, b = pixels[x, y]
            print(f"    {{ {r}, {g}, {b} }},   // (x={x}, y={y})")
        print("  },")
        print()
    print("};")


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: python png_to_array_with_coords.py input.png [array_name]")
        sys.exit(1)

    png = sys.argv[1]
    arr = sys.argv[2] if len(sys.argv) > 2 else "frame"

    png_to_c_array_with_coords(png, arr)
