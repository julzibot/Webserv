import sys
import os
import base64
from io import BytesIO
from PIL import Image

def store_base64_image(base64_string, output_directory):
    try:
        # Ensure the output directory exists, create it if not
        os.makedirs(output_directory, exist_ok=True)

        # Decode the base64 string to bytes
        image_data = base64.b64decode(base64_string)

        # Convert the bytes to a BytesIO stream
        image_stream = BytesIO(image_data)

        # Open the image using PIL (Python Imaging Library)
        image = Image.open(image_stream)

        # Extract the filename from the base64 string or provide a default name
        filename = "stored_image.png"

        # Build the output path for the stored image
        output_path = os.path.join(output_directory, filename)

        # Save the image to the output path
        image.save(output_path)

        print(f"Image stored successfully at: {output_path}")

    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    # Check if the correct number of arguments is provided
    if len(sys.argv) != 3:
        print("Usage: python store_base64_image.py <base64_string> <output_directory>")
        sys.exit(1)

    # Get the base64 string and output directory from command-line arguments
    base64_string = sys.argv[1]
    output_directory = sys.argv[2]

    # Call the function to store the base64-encoded image
    store_base64_image(base64_string, output_directory)