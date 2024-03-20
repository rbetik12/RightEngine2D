import os
import shutil
import sys

if len(sys.argv) < 2:
    print("Usage: python prepare_dist_dir.py <build_type>")
    print("<build_type>: Debug|Release")
    sys.exit(1)

build_type = sys.argv[1]
if (build_type not in "Debug" and build_type not in "Release"):
    print(f"Incorrect build type: {build_type}")
    sys.exit(1)

files_amount = 0

def copy_tree(src_dir, dst_dir, exclude_extensions = []):
    global files_amount
    # Create the destination directory if it doesn't exist
    if not os.path.exists(dst_dir):
        os.makedirs(dst_dir)

    # Iterate through all items in the source directory
    for item in os.listdir(src_dir):
        src_item = os.path.normpath(os.path.join(src_dir, item))
        dst_item = os.path.normpath(os.path.join(dst_dir, item))

        # If it's a directory, recursively call copy_tree
        if os.path.isdir(src_item):
            copy_tree(src_item, dst_item, exclude_extensions)
        # If it's a file, copy it to the destination directory
        else:
            exclude = False
            for ext in exclude_extensions:
                if os.path.splitext(src_item)[1] == ext:
                    exclude = True
                    break
            
            if exclude:
                print(f'Skipping {src_item} due to excluded extension')
                continue

            print(f'Copying {src_item} to {dst_item}')
            shutil.copy2(src_item, dst_item)
            files_amount += 1

project_resources = './Projects'
engine_resources = './Resources'
binaries = f'./.build/Win/.bin/{build_type}'

copy_tree(project_resources, '.dist/Projects')
copy_tree(engine_resources, '.dist/Resources')
copy_tree(binaries, '.dist/.bin', ['.pdb', '.log'])
copy_tree('./Scripts/ci/launch_scripts', '.dist')

print(f'Distrubution directory was successfully created with {files_amount} files')