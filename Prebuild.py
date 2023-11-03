import os
import json
import shutil
import hashlib
import subprocess

glslc_path = "C:\\SE\\Libraries\\C++\\Vulkan\\Bin\\glslc.exe"

file_paths = [f"{folder}\\{file_name}" for folder, _, file_names in os.walk("Resources\\Shaders") for file_name in file_names]
file_hashes = dict(keys=file_paths, values=[hashlib.md5(open(file_path).read().encode(), usedforsecurity=False).digest() for file_path in file_paths])
shader_idxs = [index for index, file_path in enumerate(file_hashes) if file_path.count("\\Shaders\\")]

#json_data = json.load(open("ResourceHashes.json"))

cwd = os.getcwd() + '\\'

if not os.path.exists("Application//Resources"): os.mkdir("Application//Resources")
for file_path in file_paths:
    dst_file_path = f"Application\\{file_path}.cso"
    dst_dir = "Application\\" + '\\'.join(file_path.split('\\')[:-1])
    if not os.path.exists(dst_dir): os.mkdir(dst_dir)
    
    if file_path.count("\\Shaders\\"):
        result = subprocess.run([glslc_path, file_path, "-o", dst_file_path], shell = True)
        
    else:
        shutil.copyfile(file_path, dst_file_path);
