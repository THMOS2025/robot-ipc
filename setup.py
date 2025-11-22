import os
import shutil
from setuptools import setup
from setuptools.command.install import install

def get_lib_file():
    base_dir = os.path.dirname(os.path.abspath(__file__))
    build_dir = os.path.abspath(os.path.join(base_dir, 'build'))
    if os.name == 'posix':
        import platform
        if platform.system() == 'Darwin':
            lib_file = 'librobot_ipc.dylib'
        else:
            lib_file = 'librobot_ipc.so'
    else:
        lib_file = 'librobot_ipc.dll'
    candidate = os.path.join(build_dir, lib_file)
    if os.path.exists(candidate):
        return candidate
    raise FileNotFoundError(f"{candidate} not found. Please build the shared library first.")

class CustomInstall(install):
    def run(self):
        lib_file = get_lib_file()
        target_dir = os.path.join(self.install_lib, 'robot_ipc')
        self.mkpath(target_dir)
        shutil.copy2(lib_file, target_dir)
        super().run()

setup(
    name='robot_ipc',
    version='0.1.0',
    description='robot_ipc python binding',
    author='Your Name',
    packages=['robot_ipc'],
    package_dir={'robot_ipc': 'python3'}, 
    package_data={'robot_ipc': ['librobot_ipc.so', 'librobot_ipc.dylib', 'librobot_ipc.dll']},
    include_package_data=True,
    cmdclass={'install': CustomInstall},
)