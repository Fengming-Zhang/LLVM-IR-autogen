# LLVM IR Autogen

### 分析设计

- 代码框架

  本实验中，已经通过LLVM前端的词法分析、语法分析，根据输入的c1文件生成其抽象语法树。Assembly_builder.cpp需要根据已经构建好的AST，通过对每个结点(实际上只实现了部分结点)的遍历，在这个过程中创建生成LLVM IR的代码。

- 遍历过程的设计

  - 遍历入口为assembly结点，在这个根节点上递归地遍历它的global_defs中的每个部分

  - block结点：每当进入新的函数或是使用了控制流语句时(if、while)，都需要新建一个基本块。注意退出基本块时需要CreateBr或CreateRet

  - 函数：进入和退出函数时需要改变作用域（利用enter_scope()和exit_scope()方法），将in_global置为false并重设current_function(BasicBlock::Create需要用到这个全局变量)。然后对node.body进行遍历

  - 变量定义：这里几乎是处理起来最麻烦的一个结点了，需要考虑是否是全局变量、是否是数组、是否有初始化(以及初始化时是否需要进行类型转换)等等，并需要结合assembly_builder.h中的全局变量来处理变量定义时的各种情况(assign结点类似)。注意处理对数组的界限检查、非法引用等。另外数组元素有初始化时，可能initializers的长度不足array_length，此时需要用0补齐。

  - 表达式：

    - 上层：包括关系运算、算术运算、正负号运算等，这些上层的表达式直接分别遍历每个操作数，得到它们的值后根据op类型直接调用builder的方法即可，需要注意的也就是操作数类型的处理了。

    - 底层：包括lval结点和literal结点。

      literal相对比较好处理，这里主要需要注意的是要根据node.is_int的值来判断是将使用int还是float的const_result，如在constexpr_expected为true时：

      ```c
      if (node.is_int)
          int_const_result = node.intConst ;
      else
      	float_const_result = node.floatConst ;
      ```

      以及处理完后，需要将is_result_int这个全局变量赋值，来告知上层选取常量结果时是从int_const_result还是float_const_result中取。

      lval结点处理时，首先需要判断左值还是右值，以此判断是否需要取变量地址。另外要注意数组索引的合法性检查。处理数组时，需要先用CreateGEP方法来通过基地址+偏移量的方法获取当前所需元素的地址，再Load到value_result中。

  - 赋值语句：这里同变量定义部分一样，需要在赋值时考虑类型转换问题。但由于左值valuename是一个地址(PtrTy)，右值valuenum是一个TYPE32或TYPEFLOAT，因此无法直接比较类型。我没有发现合适的API，因此采用了把左值load到tmp中，再比较tmp和valuenum的类型这种笨办法。这导致最终的.ll代码中出现冗余代码（对赋值语句左值的Load是没有意义的）。

  - 控制流语句：包括函数调用、if else语句、while loop语句等，当底层实现完备时，这些上层的语句都比较容易实现（类似汇编的思路跳转即可）。因为c1语言中的函数都没有参数传递，因此函数调用语句也很简单。

综上发现，其实assembly_builder.cpp的主要处理是在类型的判断和转化上的。在代码编写过程中需要时刻考虑类型的问题，稍有不慎就会出现类型不匹配的错误。另外在底层结点访问时，需要考虑上层结点会用到哪些值，需要通过哪些全局变量来向上层传递信息(如is_result_int)。

### 遇到的问题

- 没有设置变量is_result_int，而上层赋值是根据左值的类型去选择在哪个const_result中取值，结果可能不是那个有效值。

- 开始时没有处理好float和double的关系。实验中的c1语言将float当作double类型处理，因此不用再进行double到float的转换了。
- 起初CreateLoad时多传入了一个value_result->getType()参数，结果导致IR代码中出现了load double \*这样的语句(可能由于传入的是一个PtrTy类型)，当然是不合法的。后来发现只要传入一个Value \*变量，这个方法会自动选择是load成int还是float类型。

- 最开始调试时出现了段错误，而且出这个错误时用来debug的输出代码运行了却没有输出，最后无奈使用gdb调试才发现是在Function::Create的时候多传了一个nullptr参数进去。(搞了巨久，最后也不知道为什么那个printf语句没有输出，但是很奇怪，换成std::cout就可以输出了)

其实代码基本写完，debug的过程中也就基本是在处理类型问题，逻辑上没有特别困难的地方。主要是在底层的几个结点时一定要细心。