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

#Install submodules if needed
print("Downloading submodules...")
sub.run("git submodule update --init")
print("Finished downloading of the submodules!")

#Build and install custom built packages if needed
def check_lib_version(package_name, version):
    conan_api = ConanAPI()
    recipe_ref = RecipeReference(package_name, version)
    return len(conan_api.list.recipe_revisions(recipe_ref)) > 0


print("Preparing conan packages...")
#imguizmo
if not check_lib_version("imguizmo", "1.83.1"):
    sub.run("conan create Scripts/lib/imguizmo/all -s build_type=Debug --version 1.83.1")
    sub.run("conan create Scripts/lib/imguizmo/all -s build_type=Release --version 1.83.1")

if not check_lib_version("rttr", "0.9.6.1"):
    sub.run("conan create Scripts/lib/rttr/all -s build_type=Debug --version 0.9.6.1")
    sub.run("conan create Scripts/lib/rttr/all -s build_type=Release --version 0.9.6.1")

#Generate solution and copy all prebuilt binaries to binary folder

def copy_dll_files(source_dir, destination_dir):
    if not os.path.exists(destination_dir):
        os.makedirs(destination_dir)

    copied_dlls = set()

    for root, _, files in os.walk(source_dir):
        for filename in files:
            if filename.lower().endswith(".dll"):

                if (filename in copied_dlls):
                    continue
                else:
                    copied_dlls.add(filename)

                source_path = os.path.join(root, filename)
                destination_path = os.path.join(destination_dir, filename)

                try:
                    shutil.copy2(source_path, destination_path)
                    print(f"Copied '{filename}' to '{destination_dir}'.")
                except Exception as e:
                    print(f"Error copying '{filename}': {e}")


print(f"Generating solution...")
sub.run(f"conan install . --deployer=dll_deployer --output-folder=.build/lib --build=missing --profile=win-64 -s build_type={build_type}")

copy_dll_files(".build/lib/dll", f".build/Win/.bin/{build_type}")

try:
    completed_process = sub.run(f"cmake -B .build/Win -DCMAKE_BUILD_TYPE={build_type} --preset conan-default .", shell=True, check=True, text=True)
    
    if completed_process.returncode == 0:
        print("Solution was successfully generated!")
    else:
        print("Error: Solution generation failed")
        print("Error output:", completed_process.stderr)
except sub.CalledProcessError as e:
    print("Error while generating the solution:", e)
except Exception as e:
    print("An unexpected error occurred:", e)
