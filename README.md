# compiler
2024年全国大学生计算机系统能力大赛-编译系统设计赛-编译系统实现赛项目

队伍学校：合肥工业大学(宣城校区)

队伍名称：

队伍成员：

## 项目简介

这是一个 SysY 2022语言的编译器，目标平台是星光2。
## 构建

进入项目根目录然后执行：

```bash
xmake build
```

### SysY 语言定义与运行时库

* [SysY 语言定义](https://gitlab.eduxiji.net/csc1/nscscc/compiler2023/-/blob/master/SysY2022语言定义-V1.pdf)
* [SysY 运行时库定义](https://gitlab.eduxiji.net/csc1/nscscc/compiler2023/-/blob/master/SysY2022运行时库-V1.pdf )

### SysY 语言测试用例

可以到大赛官方网站上下载：[SysY 语言测试用例](https://gitlab.eduxiji.net/csc1/nscscc/compiler2023/-/tree/master/%E5%85%AC%E5%BC%80%E6%A0%B7%E4%BE%8B%E4%B8%8E%E8%BF%90%E8%A1%8C%E6%97%B6%E5%BA%93)

## 编译器的中间代码优化部分的设计

中间代码部分包含二个层次的 IR
### 中层 IR

设计上接近 LLVM IR，适合于各类通用优化。

* liveVar: 活跃变量分析
* combinebb：块合并
* instrcombine：指令合并
* sccp：常量折叠
* DCE：死代码删除
* DPE：循环phi删除
* Dominators：支配信息分析
* FuncInline：函数内联
* G2L：全局变量局部化
* LoopInvariant：循环不变量
* LICM：循环不变量外提
* Mem2Reg：构造 SSA 形式 IR
* valnumbering：值编号
* 等等
### 低层 IR

设计上贴近riscv isa，与后端相配合共同完成指令选择。

* breakGEP：将 GEP 指令拆分为子指令
* Cmbinejj：将比较指令和分支指令合并,方便后端指令生成
* loadimm：由于riscv架构对于立即数限制较多,故将部分立即数提升为寄存器

## 编译器的后端部分的设计

