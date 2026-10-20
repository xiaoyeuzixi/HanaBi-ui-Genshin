# HanaBi UI · 甘雨主题

HanaBi UI 是使用 **C++20、Win32、Direct3D 11 和 Dear ImGui** 开发的 Windows 原生桌面界面展示项目。整体围绕甘雨的冰元素、蓝白配色与动态背景设计，包含冰晶雪花开场、主题切换、窗口动效和多页面交互。

**交流群：835775657**，可在 QQ 中搜索群号加入。

## 项目定位

当前版本用于展示界面布局、视觉风格、控件交互与动画效果。自瞄、视觉、调节、杂项等页面中的参数属于界面演示数据，尚未连接实际业务后端或游戏功能。

主要面向以下场景：

- Windows 原生桌面 UI 的布局与交互参考。
- 甘雨主题的视觉表现、动态背景和冰晶材质展示。
- 自定义无边框窗口、主题系统和动画衔接的实现参考。

## 主要功能

### 冰晶雪花开场

- 使用深蓝冰晶材质表现雪花的层次、折射、边缘高光与辉光。
- 结合旋转、粒子和背景运动形成开场动画。
- 开场结束时，雪花平滑缩小并移动至侧栏 Logo 位置，随后作为标识持续显示。
- 开场采用独立的动画时间线，包含完成和跳过时的衔接处理。

### 主题与界面

- 提供甘雨、白色、黑色三种主题。
- 甘雨主题包含动态背景、透明卡片、冰蓝描边和配套窗口按钮。
- 包含自瞄、视觉、调节、杂项、主题五个页面。
- 支持开关、滑块、分段选项、按键录入和状态提示等交互。
- 使用极简无脸模特展示目标部位选择和视觉预览。
- 主题页可调整整体字号，范围为 **100%–125%**，默认 **112%**。

### 窗口与动效

- 支持拖动窗口、边缘缩放、最大化、最小化和关闭。
- 启动时先完成首帧准备，再进行约 **0.24 秒**的平滑显现，避免初始化期间暴露空白窗口。
- 最小化采用约 **0.36 秒**的内收、下沉和淡出动画，恢复采用约 **0.30 秒**的反向过渡。
- 页面、主题和控件采用插值与缓动处理。
- 根据显示器尺寸和 DPI 调整初始窗口大小与界面缩放。

## 技术栈

| 技术 | 用途 |
| --- | --- |
| C++20 | 应用逻辑、资源管理、动画与界面实现 |
| Win32 API | 窗口创建、消息循环、拖动、缩放与系统窗口操作 |
| Direct3D 11 | GPU 渲染、纹理处理、背景效果与离屏目标 |
| DXGI | 交换链、画面呈现与帧延迟控制 |
| HLSL / D3DCompiler | 冰晶材质与动态背景着色器 |
| Dear ImGui | 控件、文本和自定义绘制，使用 Win32 / DX11 后端 |
| stb_image | PNG、JPG 等图片资源解码 |
| Windows 资源系统 | 将字体、贴图等资源通过 `RCDATA` 编译进程序 |
| Visual Studio 2022 / MSBuild | C++ 工程管理、资源编译与链接 |

渲染流程使用垂直同步和帧延迟控制；开场等绘制在设备支持时使用 4 倍 MSAA。背景、冰晶材质和普通界面分别组织，再组合为最终画面。

## 目录结构

```text
HanaBi-ui-Genshin/
├── HanaBi.sln                 解决方案
├── HanaBi.vcxproj             主程序工程
├── HanaBi.vcxproj.filters     Visual Studio 功能筛选器
├── hanabi.rc                  内嵌资源清单
├── src/
│   ├── main.cpp               程序入口、窗口消息与渲染循环
│   ├── assets/                内嵌资源读取和字体加载
│   ├── core/                  时间、缓动、弹簧和显示适配
│   ├── gfx/                   纹理、动态背景和窗口动画
│   ├── intro/                 开场时间线与冰晶材质
│   └── ui/                    主题、绘制、控件和页面布局
├── assets/
│   ├── fonts/                 字体及已处理的字体子集
│   └── tex/                   最终使用的图片和效果贴图
├── third_party/
│   ├── imgui/                 Dear ImGui 及 Win32 / DX11 后端
│   └── stb/                   stb_image
├── .gitignore
└── README.md
```

工程保持 **一个解决方案、一个主程序项目**。ImGui 的必要源码直接参与主程序编译；Visual Studio 中按界面、开场动画、渲染、基础、资源和第三方依赖等筛选器分组。

## 开发与运行环境

- Windows 10 / Windows 11，x64。
- 支持 Direct3D 11、Feature Level 11.0 的显卡与驱动。
- Visual Studio 2022，安装“使用 C++ 的桌面开发”。
- MSVC v143 工具集与 Windows SDK（10.0 系列）。

构建所需的依赖源码、字体子集和最终图片均包含在仓库中。图片与字体已预先处理，编译时直接使用这些资源。

## 编译与运行

### 使用 Visual Studio

1. 打开 `HanaBi.sln`。
2. 选择 `Release` 和 `x64`。
3. 生成解决方案。
4. 运行 `bin\hanabi.exe`。

也可以选择 `Debug | x64` 进行调试。

### 使用命令行

在 Visual Studio 2022 的 Developer PowerShell 中进入项目根目录：

```powershell
msbuild .\HanaBi.sln /m /p:Configuration=Release /p:Platform=x64
.\bin\hanabi.exe
```

本仓库的工程输出固定为 **`bin\hanabi.exe`**。Debug 和 Release 使用同一个程序名称与输出位置，后续编译覆盖该文件；中间文件与调试符号放在 `obj\HanaBi\` 中。

字体和运行时贴图已嵌入可执行文件，可将生成的 `hanabi.exe` 单独复制至其他目录运行。

## 操作说明

- 拖动顶部非按钮区域可以移动窗口，拖动边缘可以调整大小。
- 右上角提供最小化、最大化 / 还原和关闭按钮。
- 左侧栏切换页面，在主题页选择外观、调整字号与相关视觉选项。
- 开场期间按 `Esc` 可以跳过开场并过渡到主界面。
- 页面中的配置和参数主要用于当前运行期间的交互展示。

## 验证方式

程序提供原生窗口生命周期的诊断入口：

```powershell
.\bin\hanabi.exe --exercise-native-window --theme 0 --page 4 --font-percent 120
```

其中 `--theme 0`、`1`、`2` 分别对应甘雨、白色和黑色主题。诊断会自动检查首帧准备、启动显现、普通及最大化状态下的最小化与恢复，以及窗口尺寸、样式和页面状态的保持情况。诊断窗口由系统合成器隐藏，完成后自动退出。

## 仓库与版本管理

仓库上传源码、工程配置、依赖和必需资源。`.vs/`、`bin/`、`obj/`、预览帧图、用户本机设置及本地凭据文件由 `.gitignore` 排除。

默认分支为 `main`。提交时按实际变更选择文件，例如：

```powershell
git add README.md src/ui/shell.cpp
git diff --cached --check
git diff --cached
git commit -m "Update UI layout and documentation"
git push origin main
```

第三方依赖的授权说明请参考 [ImGui LICENSE](third_party/imgui/LICENSE.txt)、[stb_image.h](third_party/stb/stb_image.h) 及资源目录中随附的授权文件。

## 交流

**QQ 交流群：835775657**

欢迎交流 Windows 原生界面开发、ImGui 自定义绘制、甘雨主题设计和动画实现。
