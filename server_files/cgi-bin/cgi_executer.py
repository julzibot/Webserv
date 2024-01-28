import sys
import json
import glob

# Get the request method from the first command line argument
method = sys.argv[1].upper()

while True:
    print ('forever and ever...')


# if method == 'GET':
#     # Get all .record files in the current directory
#     files = glob.glob('*.record')

#     # Read and print the contents of each file
#     for filename in files:
#         with open(filename, 'r') as f:
#             student = json.load(f)
#             print(student)

# elif method == 'POST':
#     # Parse the student record from the second command line argument
#     student = json.loads(sys.argv[2])

#     # Store the student record in a file named "{id}.record"
#     with open(f"{student['id']}.record", 'w') as f:
#         json.dump(student, f)

# else:
#     print("Invalid request method")