#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>

#include <memory>

#define CONST(num) \
 	ConstantInt::get(context, APInt(32, num))  //得到常数值的表示,方便后面多次用到

#define CONSTFLOAT(num) \
	ConstantFP::get(context, APFloat(num))
using namespace llvm;

int main()
{
    LLVMContext context;
	Type *TYPE32 = Type::getInt32Ty(context);   
	Type *FLOAT32 = Type::getFloatTy(context) ; 
	IRBuilder<> builder(context);
	
	// fib module
	auto module = new Module("fib", context) ;
	
	// vector for function(arg)
	std::vector<Type*> Ints(1, TYPE32) ;
	auto fibFun = Function::Create(FunctionType::get(TYPE32, Ints, false), 
									GlobalValue::LinkageTypes::ExternalLinkage,
									"fib", module) ;
									
	//BasicBlock
	auto fibBB = BasicBlock::Create(context, "entry", fibFun) ;
	builder.SetInsertPoint(fibBB) ;		// start the BB
	auto retAlloca = builder.CreateAlloca(TYPE32) ;		// return allocate
	auto nAlloca = builder.CreateAlloca(TYPE32) ;		// arg n allocate
	std::vector<Value *> args;  //获取fib函数的参数,通过iterator,save in args
	for (auto arg = fibFun->arg_begin(); arg != fibFun->arg_end(); arg++) 
	{
		args.push_back(arg);
	}
	builder.CreateStore(args[0], nAlloca) ;
	
	auto nLoad = builder.CreateLoad(nAlloca) ;			// load n
	auto rAlloca = builder.CreateAlloca(TYPE32)	; 		// int r
	
	auto trueBB = BasicBlock::Create(context, "trueBB", fibFun) ;
	auto falseBB = BasicBlock::Create(context, "falseBB", fibFun) ;
	auto false_trueBB = BasicBlock::Create(context, "false_trueBB", fibFun) ;
	auto false_falseBB = BasicBlock::Create(context, "false_falseBB", fibFun) ;
	auto retBB = BasicBlock::Create(context, "", fibFun) ;
	
	auto cmpn_0 = builder.CreateICmpEQ(nLoad,CONST(0)) ;
	builder.CreateCondBr(cmpn_0, trueBB, falseBB) ;
	
	// if true
	builder.SetInsertPoint(trueBB) ;
	builder.CreateStore(CONST(0), rAlloca) ;
	builder.CreateBr(retBB) ;
	
	// else
	builder.SetInsertPoint(falseBB) ;
	auto cmpn_1 = builder.CreateICmpEQ(nLoad,CONST(1)) ;
	builder.CreateCondBr(cmpn_1, false_trueBB, false_falseBB) ;
	
	// else if (n == 1)
	builder.SetInsertPoint(false_trueBB) ;
	builder.CreateStore(CONST(1), rAlloca) ;
	builder.CreateBr(retBB) ;
	
	// else else
	builder.SetInsertPoint(false_falseBB) ;
	auto sub1 = builder.CreateNSWSub(nLoad, CONST(1)) ;
	auto call1 = builder.CreateCall(fibFun, {sub1}) ;
	auto sub2 = builder.CreateNSWSub(nLoad, CONST(2)) ;
	auto call2 = builder.CreateCall(fibFun, {sub2}) ;
	auto sum = builder.CreateNSWAdd(call1, call2) ;
	builder.CreateStore(sum, rAlloca) ;
	builder.CreateBr(retBB) ;
	
	// ret
	builder.SetInsertPoint(retBB) ;
	auto retLoad = builder.CreateLoad(rAlloca) ;
	builder.CreateStore(retLoad, retAlloca) ;
	builder.CreateRet(retLoad) ;
	
	// main
	auto mainFun = Function::Create(FunctionType::get(TYPE32, false),
                                  GlobalValue::LinkageTypes::ExternalLinkage,
                                  "main", module);
  	auto mainBB = BasicBlock::Create(context, "entry", mainFun);
	auto judgeBB = BasicBlock::Create(context, "judgeBB", mainFun) ;
	auto retmainBB = BasicBlock::Create(context, "", mainFun) ;
	auto loopBB = BasicBlock::Create(context, "loopBB", mainFun) ;

	// enter the mainBB
	builder.SetInsertPoint(mainBB) ;
	auto xAlloca = builder.CreateAlloca(TYPE32) ;
	builder.CreateStore(CONST(0), xAlloca) ;
	auto main_nAlloca = builder.CreateAlloca(FLOAT32) ;
	builder.CreateStore(CONSTFLOAT(float(8.0)), main_nAlloca) ;
	// auto main_nAlloca = builder.CreateAlloca(TYPE32) ;
	// builder.CreateStore(CONST(8), main_nAlloca) ;

	//for
	auto iAlloca = builder.CreateAlloca(TYPE32) ;
	builder.CreateStore(CONST(1), iAlloca) ;
	auto main_nLoad_float = builder.CreateLoad(main_nAlloca) ;
	auto main_nLoad = builder.CreateFPToUI(main_nLoad_float, TYPE32) ;
	// auto main_nLoad = builder.CreateLoad(main_nAlloca) ;
	builder.CreateBr(judgeBB) ;

	// judge
	builder.SetInsertPoint(judgeBB) ;
	auto iLoad = builder.CreateLoad(iAlloca) ;
	auto cmpi_n = builder.CreateICmpSLT(iLoad, main_nLoad) ;
	builder.CreateCondBr(cmpi_n, loopBB, retmainBB) ;

	// enter loop
	builder.SetInsertPoint(loopBB) ;
	auto xLoad = builder.CreateLoad(xAlloca) ;
	auto callfib = builder.CreateCall(fibFun, {iLoad}) ;
	auto addxsum = builder.CreateNSWAdd(xLoad, callfib) ;
	builder.CreateStore(addxsum, xAlloca) ;
	auto addisum = builder.CreateNSWAdd(iLoad, CONST(1)) ;
	builder.CreateStore(addisum, iAlloca) ;
	builder.CreateBr(judgeBB) ;

	// retmainBB
	builder.SetInsertPoint(retmainBB) ;
	auto retmainLoad = builder.CreateLoad(xAlloca) ;
	builder.CreateRet(retmainLoad) ;
	
    // // Just an example
    // auto module = new Module("foo_dead_recursive_loop", context);

    // auto func = Function::Create(FunctionType::get(Type::getVoidTy(context), std::vector<Type *>(), false),
    //                              GlobalValue::LinkageTypes::ExternalLinkage,
    //                              "foo", module);
    // auto bb = BasicBlock::Create(context, "entry", func);
    // builder.SetInsertPoint(bb);
    // builder.CreateCall(func, {});
    // builder.CreateRetVoid();
    // builder.ClearInsertionPoint();

    module->print(outs(), nullptr);
    delete module;
    return 0;
}
