import sys
import os
import codecs
import io
from io import BytesIO
from email import policy
from email.parser import BytesParser
from email import message_from_binary_file

def store_base64_image(base64_string, output_directory):
    try:
        # Ensure the output directory exists, create it if not
        os.makedirs(output_directory, exist_ok=True)

        # Decode the base64 string to bytes
        image_data = codecs.decode(base64_string.encode('utf-8'), 'base64')

        # Create a BytesIO stream from the decoded bytes
        image_stream = io.BytesIO(image_data)

        # Use email.message_from_binary_file to parse the image
        # (this helps identify the image format, e.g., JPEG, PNG)
        image_message = message_from_binary_file(image_stream, policy=policy.default)

        # Get the image file extension (assuming the image is in a common format)
        image_ext = image_message.get_content_type().split('/')[-1]

        # Extract the filename from the base64 string or provide a default name
        filename = "stored_image." + image_ext

        # Build the output path for the stored image
        output_path = os.path.join(output_directory, filename)

        # Save the image to the output path
        with open(output_path, 'wb') as output_file:
            output_file.write(image_data)

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