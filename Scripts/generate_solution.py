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

print(f"Generating solution...")
sub.run(f"conan install . --output-folder=.build/lib --build=missing --profile=win-64 -s build_type={build_type}")
sub.run(f"cmake -B .build/Win -DCMAKE_BUILD_TYPE={build_type} --preset conan-default .")
print("Solution was successfully generated!")