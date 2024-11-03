import subprocess as sub

print("Configuring python env")
sub.run("pip install -r Scripts/requirements.txt")

print("Preparing conan env")
sub.run("conan config install -t dir Scripts/conan")