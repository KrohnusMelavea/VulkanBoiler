import os

def getFileAttributes(file_path:str) -> tuple[int, int]:
    with open(file_path, "r") as file_handler:
        data = file_handler.read()
        return len(data), data.count('\n')

files = "Prebuild.py", "PrebuildDebug.py"
directories = "Sources", "Resources/Shaders"

total_size = 0
total_lines = 0

for file in files:
    file_size, file_lines = getFileAttributes(file)
    total_size += file_size
    total_liens = file_lines

for directory in directories:
    for folder, _, file_names in os.walk(directory):
        for file_name in file_names:
            file_size, file_lines = getFileAttributes(f"{folder}/{file_name}")
            total_size += file_size
            total_lines += file_lines

print(f"Size: {total_size}")
print(f"Lines: {total_lines}")
input()
