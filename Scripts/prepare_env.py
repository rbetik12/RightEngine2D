import subprocess as sub

print("Configuring python env")
sub.run("pip install conan==2.0.9")

print("Preparing conan env")
sub.run("conan config install -t dir Scripts/conan")