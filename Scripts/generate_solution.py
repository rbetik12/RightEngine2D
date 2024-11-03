import subprocess as sub
import sys
import os
import shutil
from conan.api.conan_api import ConanAPI
from conan.api.model import RecipeReference

if len(sys.argv) < 2:
    print("Usage: python generate_solution.py <build_type>")
    print("<build_type>: Debug|Release")
    sys.exit(1)

build_type = sys.argv[1]
if (build_type not in "Debug" and build_type not in "Release"):
    print(f"Incorrect build type: {build_type}")
    sys.exit(1)

#Remove previous cache files
try:
    os.remove(".build/Win/CMakeCache.txt")
    shutil.rmtree(".build/Win/CMakeFiles")
except Exception as e:
    pass

#Build and install custom built packages if needed
def check_lib_version(package_name, version):
    conan_api = ConanAPI()
    recipe_ref = RecipeReference(package_name, version)
    return len(conan_api.list.recipe_revisions(recipe_ref)) > 0


print("Preparing conan packages...")
if not check_lib_version("imguizmo", "1.83.2"):
    sub.run("conan create Scripts/lib/imguizmo/all -s build_type=Debug --version 1.83.2")
    sub.run("conan create Scripts/lib/imguizmo/all -s build_type=Release --version 1.83.2")

if not check_lib_version("rttr", "0.9.6.2"):
    sub.run("conan create Scripts/lib/rttr/all -s build_type=Debug --version 0.9.6.2")
    sub.run("conan create Scripts/lib/rttr/all -s build_type=Release --version 0.9.6.2")

if not check_lib_version("glslang", "1.3.296.0"):
    sub.run("conan create Scripts/lib/glslang/all -s build_type=Debug --version 1.3.296.0")
    sub.run("conan create Scripts/lib/glslang/all -s build_type=Release --version 1.3.296.0")

if not check_lib_version("spirv-cross", "1.3.296.0"):
    sub.run("conan create Scripts/lib/spirv-cross/all -s build_type=Debug --version 1.3.296.0")
    sub.run("conan create Scripts/lib/spirv-cross/all -s build_type=Release --version 1.3.296.0")

if not check_lib_version("spirv-tools", "1.3.296.0"):
    sub.run("conan create Scripts/lib/spirv-tools/all -s build_type=Debug --version 1.3.296.0")
    sub.run("conan create Scripts/lib/spirv-tools/all -s build_type=Release --version 1.3.296.0")

#Generate solution and copy all prebuilt binaries to binary folder

def copy_files(source_dir, destination_dir, extension=".dll"):
    if not os.path.exists(destination_dir):
        os.makedirs(destination_dir)

    copied_files = set()

    for root, _, files in os.walk(source_dir):
        for filename in files:
            if filename.lower().endswith(extension):

                if (filename in copied_files):
                    continue
                else:
                    copied_files.add(filename)

                source_path = os.path.join(root, filename)
                destination_path = os.path.join(destination_dir, filename)

                try:
                    shutil.copy2(source_path, destination_path)
                    print(f"Copied '{filename}' to '{destination_dir}'.")
                except Exception as e:
                    print(f"Error copying '{filename}': {e}")
                    sys.exit(1)

def check_process_status_code(code, proc_stderr):
    if code != 0:
        print("Error: Solution generation failed")
        print("Error message: ", proc_stderr)
        sys.exit(1)

print(f"Generating solution...")

try:
    status_code = sub.run(f"conan install . --deployer=dll_deployer -c tools.cmake.cmake_layout:build_folder=\".build/Win\" --build=missing --profile=win-64 -s build_type={build_type}", shell=True, check=True, text=True)
    check_process_status_code(status_code.returncode, status_code.stderr)

    copy_files(".build/Win/dll", f".build/Win/.bin/{build_type}", ".dll")
    copy_files("Scripts/bin", f".build/Win/.bin/{build_type}", ".exe")

    status_code = sub.run(f"cmake -B .build/Win -DCMAKE_BUILD_TYPE={build_type} --preset conan-default .", shell=True, check=True, text=True)
    check_process_status_code(status_code.returncode, status_code.stderr)
    
    print("Solution was successfully generated!")
    
except sub.CalledProcessError as e:
    print("Error while generating the solution:", e)
    sys.exit(1)
except Exception as e:
    print("An unexpected error occurred:", e)
    sys.exit(1)
