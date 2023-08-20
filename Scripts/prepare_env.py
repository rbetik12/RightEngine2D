import subprocess as sub

print("Preparing conan env...")
sub.run("conan config install -t dir Scripts/conan")
print("Conan env is configured!")