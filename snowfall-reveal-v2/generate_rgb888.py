import os
from PIL import Image
import numpy as np

FRAMES_DIR = "snowfall_frames"   # folder containing frame_01.png ... frame_30.png
OUTPUT_FILE = "animationFrames.h"
WIDTH = 80
HEIGHT = 80

def main():
    # get sorted frame list
    files = sorted([f for f in os.listdir(FRAMES_DIR) if f.endswith(".png")])
    num_frames = len(files)

    print(f"Found {num_frames} frames")

    header_lines = []
    header_lines.append("#ifndef ANIMATION_FRAMES_H")
    header_lines.append("#define ANIMATION_FRAMES_H\n")
    header_lines.append(f"#define NUM_FRAMES {num_frames}\n")
    header_lines.append(f"const uint8_t animationFrames[NUM_FRAMES][{HEIGHT}][{WIDTH}][3] = {{")

    for fname in files:
        path = os.path.join(FRAMES_DIR, fname)
        img = Image.open(path).convert("RGB").resize((WIDTH, HEIGHT))
        arr = np.array(img)

        header_lines.append("{")  # start frame

        for y in range(HEIGHT):
            row_values = []
            for x in range(WIDTH):
                r, g, b = arr[y, x]
                row_values.append(f"{{{r},{g},{b}}}")
            header_lines.append("  {" + ", ".join(row_values) + "},")
        
        header_lines.append("},")  # end frame

    header_lines.append("};")
    header_lines.append("#endif")

    with open(OUTPUT_FILE, "w") as f:
        f.write("\n".join(header_lines))

    print(f"Generated {OUTPUT_FILE} successfully!")

if __name__ == "__main__":
    main()
