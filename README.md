# HanaBi UI

**以《原神》角色为灵感的 Windows 原生桌面 UI 项目。**

HanaBi 将动态壁纸、角色装饰、元素材质与桌面交互结合在一起。每位角色都有对应的主题配色、窗口按钮、侧栏纹饰、主题卡片及专属开场，让启动演出与日常界面保持连贯。

目前包含 **七套角色主题** 和 **白色、黑色两套基础外观**。功能页以本地 UI 与交互展示为主，使用演示数据，尚未接入业务后端。

**QQ 交流群：835775657**

## 角色主题

| 角色 | 元素 | 界面与开场设计方向 | 设计文档 |
| --- | --- | --- | --- |
| 甘雨 Ganyu | 冰 | 深层蓝冰、清心花纹、孔明灯；冰晶花苞展开为雪花 | [ganyu](src/characters/ganyu/ganyu_design.md) |
| 八重神子 Yae Miko | 雷 | 绯樱薄绢、狐面与金饰；杀生樱唤起三瓣雷晶并汇聚 | [yae_miko](src/characters/yae_miko/yae_miko_design.md) |
| 宵宫 Yoimiya | 火 | 暖金烟火、金鱼与祭典绳饰；引线火种舒展为火元素 | [yoimiya](src/characters/yoimiya/yoimiya_design.md) |
| 纳西妲 Nahida | 草 | 新芽透玉、月莲与叶纹；生命种子展开，叶形殿堂随之显现 | [nahida](src/characters/nahida/nahida_design.md) |
| 芙宁娜 Furina | 水 | 水色帷幕、银边缎带与礼帽；帷幕打开，水元素从水上舞台升起 | [furina](src/characters/furina/furina_design.md) |
| 珐露珊 Faruzan | 风 | 青金机关、古卷与几何切角；机关聚合、展开并释放风能 | [faruzan](src/characters/faruzan/faruzan_design.md) |
| 钟离 Zhongli | 岩 | 玄金磐石、岩契回纹与印章；天星岩核打开，岩片拼合为元素标识 | [zhongli](src/characters/zhongli/zhongli_design.md) |

角色背景使用预处理纹理、遮罩与分层动画资源，由原生渲染器播放。白色与黑色是独立基础外观，不显示角色壁纸。各角色方案的具体要求及打磨进度记录在对应设计文档中。

## 交互与动画

- **主题库**：按元素筛选，以长方形卡片展示姓名及主题短句，支持滚动扩展。
- **启动选择**：跟随当前主题、固定指定主题，或随机选择角色；随机模式排除黑白基础外观。
- **外观记忆**：保存主题、字号、壁纸强度及动画选项，下次启动继续使用。
- **文字调节**：字号支持 100%–125%，默认 112%。
- **窗口操作**：支持拖动、边缘缩放、最小化、最大化、恢复和关闭，配有窗口过渡动画。
- **开场衔接**：角色开场统一为 6.20 秒，同一元素标识平滑停驻侧栏，支持中途跳过。
- **功能演示**：提供开关、滑块、选项切换、按键录入和极简无脸模特部位选择。

外观设置保存在 `%LOCALAPPDATA%\HanaBi\settings.ini`。

## 技术栈

| 技术 | 用途 |
| --- | --- |
| C++20 | 应用逻辑、资源管理、角色演出与偏好设置 |
| Win32 API | 原生窗口、输入与桌面交互 |
| Direct3D 11 / DXGI | 图形渲染与画面呈现 |
| HLSL | 冰晶折射、元素能量、辉光、水面倒影及背景效果 |
| Dear ImGui | 控件交互和自定义界面绘制 |
| stb_image | 图片解码 |
| Windows RCDATA | 将字体、图片及预处理动画资源嵌入程序 |

仓库包含可直接构建的 C++ 源码、必要依赖和预处理资源。**构建与运行无需 Python、资源生成脚本或原始壁纸工程。**

## 构建与运行

环境要求：Windows 10 / 11 x64、支持 Direct3D 11 的显卡、Visual Studio 2022，以及“使用 C++ 的桌面开发”、MSVC v143 和 Windows SDK 10.0。

1. 打开 `HanaBi.sln`。
2. 选择 `Release | x64`，生成解决方案。
3. 运行 `bin\Release\hanabi.exe`。

也可在 Visual Studio Developer PowerShell 中执行：

```powershell
msbuild .\HanaBi.sln /m /p:Configuration=Release /p:Platform=x64
.\bin\Release\hanabi.exe
```

项目只有一个解决方案和一个主程序工程，后续构建覆盖同名 `hanabi.exe`。运行资源嵌入程序，生成的 EXE 可单独运行。构建产物、缓存、调试帧和本机设置不进入 Git 版本管理。

## 源码与文档组织

```text
HanaBi.sln                 解决方案
HanaBi.vcxproj             C++ 工程
HanaBi.vcxproj.filters     Visual Studio 分类筛选器
hanabi.rc                  内嵌资源清单
src/
  characters/              按角色英文名组织
    ganyu/
      ganyu_ui.cpp         配色、窗口按钮、侧栏和主题卡片
      ganyu_intro.cpp      角色专属开场
      ganyu_design.md      设计要求、资源入口与验收记录
    yae_miko/              其余角色沿用同样结构
    yoimiya/
    nahida/
    furina/
    faruzan/
    zhongli/
  assets/                  资源读取与字体加载
  core/                    基础工具与偏好设置
  gfx/                     背景、人物动态与窗口动效
  intro/                   共享元素几何、材质和开场基础绘制
  ui/                      公共布局、控件与主题过渡
assets/
  fonts/                   字体与子集
  tex/                     已预处理的图片、遮罩和人物动画包
docs/                      公共设计要求和角色维护索引
third_party/               ImGui 与 stb_image
```

Visual Studio 中的代码、文档和角色资源同步按筛选器分类。新增同元素角色可复用元素材质，同时独立维护角色 UI 与开场。

- [角色目录与维护索引](docs/角色主题索引.md)：七个角色的实现入口与新增流程。
- [UI 设计要求](docs/UI设计要求.md)：布局、背景、窗口交互及主题库规范。
- [开场动画设计要求](docs/开场动画设计要求.md)：成形连续性、角色反馈和验收标准。

## 第三方组件

- [Dear ImGui 许可证](third_party/imgui/LICENSE.txt)
- [Source Sans 3 字体许可证](assets/fonts/SourceSans3-LICENSE.md)
- stb_image 的许可声明见 [源码](third_party/stb/stb_image.h)。
