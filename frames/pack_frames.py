import struct
from pathlib import Path


FRAME_COUNT = 6569

FRAMES_DIR = Path("frames")
OUTPUT_FILE = FRAMES_DIR / "frames.bin"

MAGIC = b"BAAS"
VERSION = 1

HEADER_FORMAT = "<4sII"
INDEX_FORMAT = "<II"

HEADER_SIZE = struct.calcsize(HEADER_FORMAT)
INDEX_SIZE = struct.calcsize(INDEX_FORMAT)


def main():
  print("Packing frames...")

  frames = []

  for frame_number in range(1, FRAME_COUNT + 1):
    path = FRAMES_DIR / f"BA{frame_number:04d}.txt"

    if not path.exists():
      raise FileNotFoundError(f"Frame not found: {path}")

    data = path.read_bytes()
    frames.append(data)

    print(f"\rFrame {frame_number:04d}/{FRAME_COUNT}", end="")

  print()

  index_offset = HEADER_SIZE
  data_offset = (HEADER_SIZE + FRAME_COUNT * INDEX_SIZE)
  offsets = []
  current_offset = data_offset

  for data in frames:
    size = len(data)

    offsets.append((current_offset, size))

    current_offset += size

  with open(OUTPUT_FILE, "wb") as file:

    # Header
    file.write(struct.pack(HEADER_FORMAT, MAGIC, VERSION, FRAME_COUNT))

    # Index
    for offset, size in offsets:
      file.write(struct.pack(INDEX_FORMAT, offset, size))

    # Frame data
    for data in frames:
        file.write(data)

  print(f"Created: {OUTPUT_FILE}")
  print(f"Size: {OUTPUT_FILE.stat().st_size / 1024 / 1024:.2f} MB")


if __name__ == "__main__":
  main()