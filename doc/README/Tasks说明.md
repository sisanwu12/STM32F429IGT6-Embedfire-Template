# Tasks.json 文件说明

本文件介绍 Tasks 中的内容以及如何编写tasks.json文件

> tasks.json 是 VS Code 的 任务配置文件，位于 .vscode/ 目录下，用来告诉 VS Code 怎样去运行外部命令行程序（比如编译、打包、运行脚本、上传固件……）。

## 本工程 tasks.json 介绍

本项目通过 Tasks 插件，将 tasks.json 中定义的每一个 task 封装为一个个 vscode 底部的按钮，便于操作。

### CMake Configure

这是第一个task，

```json
{
    "label": "CMake Configure",
    "type": "shell",
    "command": "cmake",
    "args": [
        "-DCMAKE_TOOLCHAIN_FILE=${cwd}/mcu/project/arm-gnu-none-eabi.cmake",
        "-DCMAKE_SYSTEM_NAME=Generic",
        "-DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE",
        "-GNinja",
        "-S", 
        "mcu/project",
        "-B", 
        "mcu/build"
    ],
    "options": {
        "cwd": "${workspaceFolder}"
    },
    "group": {
        "kind": "build",
        "isDefault": true
    }
}
```

这段 task 会在终端里构造出一条完整的 CMake 配置命令，为项目生成 Ninja 的构建目录。

其中

"label": "CMake Configure" // VS Code 在任务面板里显示的名字。可以任意取。

"type": "shell"  // 告诉 VS Code，用 shell 来执行这条命令。

"command": "cmake"  // 这就是要执行的程序。这里表示：调用系统里的 CMake。相当于在终端敲 cmake。

"args": [...]   // 这是给 cmake 命令追加的参数列表。

- "-DCMAKE_TOOLCHAIN_FILE=${cwd}/mcu/project/arm-gnu-none-eabi.cmake"
  
  // 告诉 CMake：使用指定的 交叉编译 toolchain 文件，即使用等号后面路径的文件里的要求来编译文件。

  // ${cwd} 是 VS Code 在任务运行时的 “current working directory”（当前目录）。这里等价于 ${workspaceFolder}。

- "-DCMAKE_SYSTEM_NAME=Generic"
  
  // 告诉 CMake，目标环境而是一个无操作系统的 MCU 环境。

- "-DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE"
  
  // 让 CMake 导出一个 compile_commands.json。这个文件非常有用：clangd / vscode C/C++ 扩展 用它来知道你的 include、宏定义、编译命令，从而：

    - 代码跳转更准确

    - 自动补全更智能

    - 错误提示更真实

- "-GNinja" 
  
  // 表示之后构建会用：**ninja** ，而不是 makefile。

- "-S", "mcu/project"
  
  // -S = 源代码目录（source）告诉 CMake：我的 CMakeLists.txt 放在 `mcu/project/ ` 这个文件夹下。

- "-B", "mcu/build"
  
  // -B = 构建目录（build）。CMake 会把生成的 Ninja 等文件全部放进 `mcu/build/` 。

- "options": { "cwd": "${workspaceFolder}" }"
  
  // 告诉 VS Code：在运行命令前，把工作目录切换到当前项目根目录。你运行 cmake 的起点就是 ${workspaceFolder}。否则相对路径可能会错。

### CMake Build

这是第二个：

```json
{
  "label": "CMake Build",
  "type": "shell",
  "command": "cmake",
  "args": [
    "--build",
    "${cwd}/mcu/build",
    "--target",
    "all"
  ],
  "options": {
    "cwd": "${workspaceFolder}"
  }
}
```

