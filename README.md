# compiler
2024年全国大学生计算机系统能力大赛-编译系统设计赛-编译系统实现赛项目

队伍学校：合肥工业大学(宣城校区)

队伍名称：决不放弃

队伍成员：牟长青、吴钦洲、焦超然、魏子尧

## 团队协作分工：
牟长青：语义分析、MIR生成、LIR生成、LIR优化、后端优化、生成汇编、测试  
吴钦洲：语义分析、MIR生成、MIR优化、测试  
焦超然：词法分析、语法分析、语义分析、MIR生成、MIR优化、LIR优化  
魏子尧：测试  
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

中间代码部分包含三个层次的 IR

### 高层IR

* AST（抽象语法树）

### 中层 IR

* 混合IR（整体上是图结构的IR，图的每一节点是线性结构的IR）

设计上接近 LLVM IR，适合于各类通用优化。

* liveVar: 活跃变量分析
* combinebb：块合并
* instrcombine：指令合并
* sccp：常量折叠
* DCE：死代码删除
* DeadPHIEli：循环phi删除
* Dominators：支配信息分析
* FuncInline：函数内联
* G2L：全局变量局部化
* LoopInvariant：循环不变量
* LICM：循环不变量外提
* LoopStrengthReduction:循环强度削弱
* LoopUnroll：循环展开
* Mem2Reg：构造 SSA 形式 IR
* valnumbering：值编号
* PureFuncCache：对部分递归函数进行缓存
* GepCombine：合并gep指令
* 等等
### 低层 IR

* 混合IR（指令结构更接近RISCV ISA的混合IR）

设计上贴近riscv isa，与后端相配合共同完成指令选择。

* breakGEP：将 GEP 指令拆分为子指令
* MemInstOffset：访存指令偏移化
* Cmbinejj：将比较指令和分支指令合并,方便后端指令生成
* loadimm：由于riscv架构对于立即数限制较多,故将部分立即数提升为寄存器

## 编译器后端的设计

### 本编译器目前支持生成的RISCV指令集为

* I（整数扩展），基础整数指集

* M（乘法和除法扩展），支持乘法和除法指令

* F（单精度浮点扩展），支持单精度浮点运算

* Zba（位操作扩展A），支持高效的位级别数据处理

### 寄存器分配

采取Linear Scan Register Allocation（线性扫描寄存器分配）

* 遍历顺序：线性扫描方法通过在程序的基本块中线性遍历指令来进行寄存器分配。它通常使用一种贪心策略，在扫描过程中为每个活跃的变量分配寄存器。

* 活动区间：在编译过程中，维护一个“活动区间”来跟踪当前程序中所有活跃的变量。活跃变量是指在程序执行过程中仍然需要使用的变量。

* 贪心策略：通过贪心算法确定哪些变量需要被分配到寄存器，哪些变量可以被溢出到内存（例如栈）中。通常，算法会选择优先保留那些在未来指令中仍然会被使用的变量。

* 分配寄存器：在每个扫描步骤中，将寄存器分配给活动集合中的变量。若寄存器不足，算法会选择将某些变量保存到内存中，并在需要时将其重新加载到寄存器中。

* 溢出处理：当活动集合中的变量超过了可用寄存器的数量时，算法会决定将某些变量的值保存在栈中（即“溢出”），以释放寄存器供其他变量使用。

### 指令选择

* 深度优先遍历IR内存对象来创建抽象汇编

* 对每一抽象汇编内存对象，根据操作数的类型等信息，打印与之匹配的RISCV汇编。


