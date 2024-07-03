//该文件里写ASM的内存构建
#ifndef ASMGEN_HPP
#define ASMGEN_HPP

#include "midend/IRVisitor.hpp"

/*需要访问的中端数据结构如下：
module
function
basicblock
instruction（不包含instruction，使用的是instruction的子类）:
binaryinst
cmpinst
fcmpinst
callinst
branchinst
returninst
getelementptrinst
storeinst
memsetinst
loadinst
allocainst
zextinst
sitofpins
fptosiinst
phiinst
cmbrinst
fcmbrinst
loadoffsetinst
storeoffsetinst
*/

class AsmGen : public IRVisitor{




};



#endif