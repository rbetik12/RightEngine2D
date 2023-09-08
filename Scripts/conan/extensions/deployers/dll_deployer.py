from conans.errors import ConanException
from conan.tools.files import copy
import os

def deploy(graph, output_folder, **kwargs):
    for name, dep in graph.root.conanfile.dependencies.items():
        if (dep.folders.package_folder is None):
            continue
        copy(graph.root.conanfile, "*.dll", dep.folders.package_folder, os.path.join(output_folder, "dll", str(dep)))