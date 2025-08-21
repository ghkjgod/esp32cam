#!/usr/bin/env python3
"""
Fix paths in compile_commands.json for VS Code IntelliSense
自动检测和修复路径映射
"""

import json
import os
import sys

def get_current_paths():
    """获取当前项目的实际路径"""
    current_project = os.path.abspath(".").replace("\\", "/")

    # 尝试从环境变量获取IDF路径
    idf_path = os.environ.get('IDF_PATH', '')
    if idf_path:
        idf_path = idf_path.replace("\\", "/")
    else:
        # 默认路径
        idf_path = "d:/workspace/esp/v5.5/esp-idf"

    # 尝试从环境变量获取工具路径
    idf_tools_path = os.environ.get('IDF_TOOLS_PATH', '')
    if idf_tools_path:
        idf_tools_path = idf_tools_path.replace("\\", "/")
    else:
        # 默认路径
        idf_tools_path = "d:/workspace/idftools"

    return current_project, idf_path, idf_tools_path

def fix_compile_commands():
    compile_commands_path = "build/compile_commands.json"

    if not os.path.exists(compile_commands_path):
        print(f"错误: {compile_commands_path} 文件不存在")
        return False

    print("读取compile_commands.json...")
    with open(compile_commands_path, 'r', encoding='utf-8') as f:
        data = json.load(f)

    print(f"找到 {len(data)} 个编译条目")

    # 获取当前实际路径
    current_project, current_idf, current_tools = get_current_paths()
    print(f"当前项目路径: {current_project}")
    print(f"当前IDF路径: {current_idf}")
    print(f"当前工具路径: {current_tools}")

    # 动态生成路径映射
    path_mappings = {
        # 常见的错误路径模式
        "E:/workspace/esp32/espcam": current_project,
        "E:/workspace/project/espcam": current_project,
        "E:/esp/v5.5/esp-idf": current_idf,
        "E:/workspace/esp/v5.5/esp-idf": current_idf,
        "E:/espidftools": current_tools,
        "E:/workspace/idftools": current_tools,
        # Windows路径格式
        "E:\\workspace\\esp32\\espcam": current_project.replace("/", "\\"),
        "E:\\workspace\\project\\espcam": current_project.replace("/", "\\"),
        "E:\\esp\\v5.5\\esp-idf": current_idf.replace("/", "\\"),
        "E:\\workspace\\esp\\v5.5\\esp-idf": current_idf.replace("/", "\\"),
        "E:\\espidftools": current_tools.replace("/", "\\"),
        "E:\\workspace\\idftools": current_tools.replace("/", "\\")
    }
    
    # Fix paths in each entry
    for entry in data:
        # Fix directory path
        for old_path, new_path in path_mappings.items():
            if old_path in entry.get("directory", ""):
                entry["directory"] = entry["directory"].replace(old_path, new_path)
            
            # Fix command paths
            if old_path in entry.get("command", ""):
                entry["command"] = entry["command"].replace(old_path, new_path)
            
            # Fix file paths
            if old_path in entry.get("file", ""):
                entry["file"] = entry["file"].replace(old_path, new_path)
            
            # Fix output paths
            if old_path in entry.get("output", ""):
                entry["output"] = entry["output"].replace(old_path, new_path)
    
    # Write back the fixed file
    print("Writing fixed compile_commands.json...")
    with open(compile_commands_path, 'w', encoding='utf-8') as f:
        json.dump(data, f, indent=2)
    
    print("✓ compile_commands.json paths fixed successfully!")
    return True

if __name__ == "__main__":
    if fix_compile_commands():
        print("\nNext steps:")
        print("1. Reload VS Code window (Ctrl+Shift+P -> 'Developer: Reload Window')")
        print("2. Or restart VS Code")
        print("3. If errors persist, run 'C/C++: Reset IntelliSense Database'")
    else:
        sys.exit(1)
