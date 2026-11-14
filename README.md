# TMD - Terminal MarkDown renderer  
### 什么是TMD?  
TMD是一个**轻量**，**快速**的**终端**Markdown渲染器。  
### TMD的环境要求?  
* 安装gcc/clang等构建工具，安装了glibc    
* Linux系统  
* 终端支持CSI特性（见[此处](https://www.commonmark.org)）  
### TMD支持的MD特性?   
TMD支持部分**CommonMark**特性和部分**GFM**特性，以及md4c库的独家特性。由于KGP的崛起，将来有望支持图片和LaTeX的渲染。  
TMD支持以下特性：  
* 粗体   
* 斜体   
* 下划线（*md4c*独家特性）    
* 删除线    
* 单行代码块    
* 高亮    
### 如何构建TMD？   
运行以下脚本：   
```bash   
git clone https://github.com/StormFishNotFish/TMD.git   
cd TMD   
mkdir build && cd build   
cmake ..   
cmake --build .   
```   
### 如何运行？   
```./tmd <path-to-your-md-file>```  
