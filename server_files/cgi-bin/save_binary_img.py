import sys
import os

def save_image(binary_data, output_path):
    try:
        # Ensure the output directory exists, create it if not
        os.makedirs(os.path.dirname(output_path), exist_ok=True)

        # Write the binary data to the specified output path
        with open(output_path, 'wb') as output_file:
            output_file.write(binary_data)

        print(f"Image saved successfully at: {output_path}")

    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    # Check if the correct number of arguments is provided
    if len(sys.argv) != 3:
        print("Usage: python save_image.py <binary_data_path> <output_path>")
        sys.exit(1)

    # Get the binary data path and output path from command-line arguments
    binary_data_path = sys.argv[1]
    output_path = sys.argv[2]

    try:
        # Read the binary data from the specified file
        with open(binary_data_path, 'rb') as binary_file:
            binary_data = binary_file.read()

        # Call the function to save the image
        save_image(binary_data, output_path)

    except FileNotFoundError:
        print(f"Error: File not found - {binary_data_path}")
    except Exception as e:
        print(f"Error: {e}")