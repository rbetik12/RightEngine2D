import subprocess as sub
import sys

if len(sys.argv) < 2:
    print("Usage: python build_solution.py <build_type>")
    print("<build_type>: Debug|Release")
    sys.exit(1)

build_type = sys.argv[1]
if (build_type not in "Debug" and build_type not in "Release"):
    print("Incorrect build type")
    sys.exit(1)


print(f"Started solution build for '{build_type}' configuration")
try:
    completed_process = sub.run(f"cmake --build .build/Win --config {build_type}", shell=True, check=True, text=True)
    
    if completed_process.returncode == 0:
        print("Solution was successfully built!")
    else:
        print("Error: Solution build failed.")
        print("Error output:", completed_process.stderr)
except sub.CalledProcessError as e:
    print("Error while building the solution:", e)
except Exception as e:
    print("An unexpected error occurred:", e)
