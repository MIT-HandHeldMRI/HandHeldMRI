import os
from PIL import Image
import pillow_heif

# Register HEIF opener with Pillow
pillow_heif.register_heif_opener()

# Loop through files in the current directory
for filename in os.listdir("."):
    if filename.lower().endswith(".heic"):
        jpg_filename = os.path.splitext(filename)[0] + ".jpg"
        try:
            with Image.open(filename) as img:
                img = img.convert("RGB")  # Ensure compatibility
                img.save(jpg_filename, "JPEG")
            print(f"Converted: {filename} -> {jpg_filename}")
        except Exception as e:
            print(f"Failed to convert {filename}: {e}")
