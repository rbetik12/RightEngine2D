import os
import subprocess

EXCLUDED_FILES = ['imgui_impl_vulkan.cpp', 'imgui_impl_glfw.cpp']

def replace_tabs_with_spaces(folder_path):
    # Iterate through all files in the specified folder
    for root, dirs, files in os.walk(folder_path):
        for file_name in files:
            file_path = os.path.join(root, file_name)

            exclude = file_name in EXCLUDED_FILES
            
            if exclude:
                continue

            if file_path.endswith(".cpp") or file_path.endswith(".hpp"):
                with open(file_path, 'r') as file:
                    file_content = file.read()
                
                # Replace tabs with 4 spaces
                modified_content = file_content.replace('\t', '    ')
                
                # Write modified content back to the file
                with open(file_path, 'w') as file:
                    file.write(modified_content)
                print(f'Formatted {file_path}')

# Specify the folder path here
folder_path = 'Src'
replace_tabs_with_spaces(folder_path)
