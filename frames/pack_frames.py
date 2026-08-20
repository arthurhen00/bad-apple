import argparse
import struct
from pathlib import Path

MAGIC = b"BAAS"
VERSION = 2

HEADER_FORMAT = "<4sIII"
INDEX_FORMAT = "<II"

HEADER_SIZE = struct.calcsize(HEADER_FORMAT)
INDEX_SIZE = struct.calcsize(INDEX_FORMAT)

def parse_args():
  parser = argparse.ArgumentParser(description="Empacota frames ASCII .txt em um arquivo _frames.bin.")
  parser.add_argument("--dir-path", type=Path, required=True, help="Pasta contendo os arquivos .txt dos frames.")
  parser.add_argument("--fps", type=int, required=True, help="FPS do vídeo.")
  return parser.parse_args()

def main():
  args = parse_args()

  frames_dir = args.dir_path
  fps = args.fps

  if not frames_dir.exists():
    raise FileNotFoundError(f"Pasta de frames não encontrada: {frames_dir}")

  if not frames_dir.is_dir():
    raise NotADirectoryError(f"O caminho informado não é uma pasta: {frames_dir}")

  if fps <= 0:
    raise ValueError("FPS deve ser maior que zero.")

  frames = sorted(frames_dir.glob("*.txt"))

  if not frames:
    raise FileNotFoundError(f"Nenhum arquivo .txt encontrado em: {frames_dir}")

  frame_count = len(frames)

  print(f"Frames found: {frame_count}")
  print(f"FPS: {fps}")
  print("Packing frames...")

  frame_data = []

  for frame_number, path in enumerate(frames, start=1):
    data = path.read_bytes()
    frame_data.append(data)

    print(f"\rFrame {frame_number:06d}/{frame_count:06d}", end="")

  print()

  index_offset = HEADER_SIZE
  data_offset = (HEADER_SIZE + frame_count * INDEX_SIZE)

  offsets = []
  current_offset = data_offset

  for data in frame_data:
    size = len(data)

    offsets.append((current_offset, size))

    current_offset += size

  output_file = frames_dir / "_frames.bin"

  with open(output_file, "wb") as file:

    # Header
    file.write(struct.pack(HEADER_FORMAT, MAGIC, VERSION, frame_count, fps))

    # Index
    for offset, size in offsets:
      file.write(struct.pack(INDEX_FORMAT, offset, size))

    # Frame data
    for data in frame_data:
      file.write(data)

  print(f"Created: {output_file}")
  print(f"Size: {output_file.stat().st_size / 1024 / 1024:.2f} MB")


if __name__ == "__main__":
  main()