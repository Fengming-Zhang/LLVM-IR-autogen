
#include "assembly_builder.h"
#include <cstdio>
#include <vector>

using namespace llvm;
using namespace c1_recognizer::syntax_tree;

#define CONST(num) \
    ConstantInt::get(context, APInt(32, num))

Type *TYPE32, *TYPEVOID, *TYPEFLOAT ;
Value *ret ;
// 用于每次判断返回的是不是ptr，如果是把ret更新为TYPE32类型
// #define ValueLoad() \
//     if(lval_as_rval) value_result = builder.CreateLoad(value_result); \
//     else value_result = builder.CreateZExt(value_result, value_result->getType());

void assembly_builder::visit(assembly &node)
{
    // std::cout << ";assembly" << std::endl ;
    // printf("Enter assembly") ;
    error_flag = 0 ;
    value_result = CONST(0) ;
    lval_as_rval = true ;
    constexpr_expected = false ;
    in_global = true ;
    TYPE32 = Type::getInt32Ty(context) ;
    TYPEVOID = Type::getVoidTy(context) ;
    TYPEFLOAT = Type::getDoubleTy(context) ;
    for (auto decl : node.global_defs)      // 遍历子树
    {
        decl->accept(*this) ;
    }
}

void assembly_builder::visit(func_def_syntax &node)
{
    // std::cout << ";func_def_syntax" << std::endl ;
    Type *rType = TYPEVOID ;
    auto Fun = llvm::Function::Create(FunctionType::get(Type::getVoidTy(context), {}, false), 
                llvm::GlobalValue::LinkageTypes::ExternalLinkage,
                node.name, module.get()) ;
    if (functions.count(node.name))
    {
        error_flag = true ;
        err.error(node.line, node.pos, "Function " + node.name + " declared!") ;
        return ;
    }
    else
    {
        functions.emplace(node.name, Fun) ;
    }
    auto entry = BasicBlock::Create(context, "", Fun) ;
    builder.SetInsertPoint(entry) ;
    in_global = false ;
    current_function = Fun ;
    node.body->accept(*this) ;
    builder.CreateRetVoid() ;
}

void assembly_builder::visit(cond_syntax &node)
{
    // std::cout << ";cond_syntax" << std::endl ;
    Value *cmp ;
    Value *left , *right ;
    node.lhs->accept(*this) ;
    if (constexpr_expected == true)
    {
        if(is_result_int == true)
        {
            auto left_value = int_const_result ;
            left = ConstantInt::get(context, APInt(32, left_value)) ;
        }
        else
        {
            auto left_value = float_const_result ;
            left = ConstantFP::get(Type::getDoubleTy(context), left_value) ;
        }
    }
    else
    {
        left = value_result ;
    }

    node.rhs->accept(*this) ;
    if (constexpr_expected == true)
    {
        if(is_result_int == true)
        {
            auto right_value = int_const_result ;
            right = ConstantInt::get(context, APInt(32, right_value)) ;
        }
        else
        {
            auto right_value = float_const_result ;
            right = ConstantFP::get(Type::getDoubleTy(context), right_value) ;
        }
    }
    else
    {
        right = value_result ;
    }
    // 进行运算
    if (left->getType()->isIntegerTy(32) && right->getType()->isIntegerTy(32))
    {
        switch (node.op)
        {
            case relop::equal:
                cmp = builder.CreateICmpEQ(left, right) ;
                break;
            case relop::non_equal:
                cmp = builder.CreateICmpNE(left, right) ;
                break;
            case relop::less:
                cmp = builder.CreateICmpSLT(left, right) ;
                break;
            case relop::less_equal:
                cmp = builder.CreateICmpSLE(left, right) ;
                break;
            case relop::greater:
                cmp = builder.CreateICmpSGT(left, right) ;
                break;
            case relop::greater_equal:
                cmp = builder.CreateICmpSGE(left, right) ;
                break;
            default:
                error_flag = 1 ;
                err.error(node.line, node.pos, "Invalid RELOP!") ;
                return;
        }
    }
    else
    {
        if (left->getType()->isIntegerTy(32))
            left = builder.CreateSIToFP(left, TYPEFLOAT) ;
        else if (right->getType()->isIntegerTy(32))
            right = builder.CreateSIToFP(right, TYPEFLOAT) ;
        switch (node.op)
        {
            case relop::equal:
                error_flag = 1 ;
                err.error(node.line, node.pos, "Can't compare two float nums!") ;
                exit(0) ;
                break;
            case relop::non_equal:
                error_flag = 1 ;
                err.error(node.line, node.pos, "Can't compare two float nums!") ;
                exit(0) ;
                break;
            case relop::less:
                cmp = builder.CreateFCmpOLT(left, right) ;
                break;
            case relop::less_equal:
                cmp = builder.CreateFCmpOLE(left, right) ;
                break;
            case relop::greater:
                cmp = builder.CreateFCmpOGT(left, right) ;
                break;
            case relop::greater_equal:
                cmp = builder.CreateFCmpOGE(left, right) ;
                break;
            default:
                error_flag = 1 ;
                err.error(node.line, node.pos, "Invalid RELOP!") ;
                return;
        }
    }
    value_result = cmp ;
    return ;
}

void assembly_builder::visit(binop_expr_syntax &node)
{
    // std::cout << ";binop_expr_syntax" << std::endl ;
    Value *left , *right ;
    node.lhs->accept(*this) ;
    if (constexpr_expected == true)
    {
        if(is_result_int == true)
        {
            auto left_value = int_const_result ;
            left = ConstantInt::get(context, APInt(32, left_value)) ;
        }
        else
        {
            auto left_value = float_const_result ;
            left = ConstantFP::get(Type::getDoubleTy(context), left_value) ;
        }
    }
    else
    {
        left = value_result ;
    }

    node.rhs->accept(*this) ;
    if (constexpr_expected == true)
    {
        if(is_result_int == true)
        {
            auto right_value = int_const_result ;
            right = ConstantInt::get(context, APInt(32, right_value)) ;
        }
        else
        {
            auto right_value = float_const_result ;
            right = ConstantFP::get(Type::getDoubleTy(context), right_value) ;
        }
    }
    else
    {
        right = value_result ;
    }

    if (left->getType()->isIntegerTy(32) && right->getType()->isIntegerTy(32))
    {
        switch (node.op)
        {
            case binop::plus:
                value_result = builder.CreateAdd(left, right) ;
                break;
            case binop::minus:
                value_result = builder.CreateSub(left, right) ;
                break;
            case binop::multiply:
                value_result = builder.CreateMul(left, right) ;
                break;
            case binop::divide:
                value_result = builder.CreateSDiv(left, right) ;
                break;
            case binop::modulo:
                value_result = builder.CreateSRem(left, right) ;
                break;
            default:
                error_flag = 1 ;
                err.error(node.line, node.pos, "Invalid BILOP!") ;
                return;
        }
    }
    else
    {
        if (left->getType()->isIntegerTy(32))
            left = builder.CreateSIToFP(left, TYPEFLOAT) ;
        else if (right->getType()->isIntegerTy(32))
            right = builder.CreateSIToFP(right, TYPEFLOAT) ;
        switch (node.op)
        {
            case binop::plus:
                value_result = builder.CreateFAdd(left, right) ;
                break;
            case binop::minus:
                value_result = builder.CreateFSub(left, right) ;
                break;
            case binop::multiply:
                value_result = builder.CreateFMul(left, right) ;
                break;
            case binop::divide:
                value_result = builder.CreateFDiv(left, right) ;
                break;
            case binop::modulo:
                error_flag = 1 ;
                err.error(node.line, node.pos, "Can't modulo with float num!") ;
                break;
            default:
                error_flag = 1 ;
                err.error(node.line, node.pos, "Invalid BILOP!") ;
                return;
        }
    }
}

void assembly_builder::visit(unaryop_expr_syntax &node)
{
    // std::cout << ";unaryop_expr_syntax" << std::endl ;
    Value *right ;
    node.rhs->accept(*this) ;
    // ValueLoad() ;
    if (constexpr_expected == true)
    {
        if(is_result_int == true)
        {
            auto right_value = int_const_result ;
            right = ConstantInt::get(context, APInt(32, right_value)) ;
        }
        else
        {
            auto right_value = float_const_result ;
            right = ConstantFP::get(Type::getDoubleTy(context), right_value) ;
        }
    }
    else
    {
        right = value_result ;
    }
    if (node.op == unaryop::plus)
        value_result = right ;
    else if (node.op == unaryop::minus)
    {
        if (right->getType()->isIntegerTy(32))
        {
            Value *Zero = CONST(0) ;
            value_result = builder.CreateSub(Zero, right) ;
        }
        else
        {
            Value *ZeroFLOAT = ConstantFP::get(Type::getDoubleTy(context), (double)0) ;
            value_result = builder.CreateFSub(ZeroFLOAT, right) ;
        }
    }
    else
    {
        error_flag = 1 ;
        err.error(node.line, node.pos, "Invalid UnaryOp!") ;
        return ;
    }
    return ;
}

void assembly_builder::visit(lval_syntax &node)
{
    // std::cout << ";lval_syntax" << std::endl ;
    if (std::get<0>(lookup_variable(node.name)) == nullptr) 
    {
		err.error(node.line, node.pos, "Variable " + node.name + " undeclared!") ;
		if (constexpr_expected == true) 
        {
			if (is_result_int)
                int_const_result = 0 ;
            else
                float_const_result = (double)0 ;           
		}
		error_flag = true;
		value_result = nullptr;
		return;
	}
	if (lval_as_rval == false) {
		// 左值，取地址
		if (node.array_index == nullptr)    // 变量
        {
			value_result = std::get<0>(lookup_variable(node.name));
		}
		else        // 数组，需要进行判断
        {
            if (std::get<2>(lookup_variable(node.name)) == false)   // 不是数组但有索引
            {
                error_flag = true ;
                err.error(node.line, node.pos, "Variable "+node.name+" isn't an array and can't be indexed!") ;
                return ;
            }
			auto tmp_const_exp = constexpr_expected;
			constexpr_expected = false;
			auto tmp_lval = lval_as_rval;
			lval_as_rval = true;
			node.array_index->accept(*this);
            lval_as_rval = tmp_lval ;
            constexpr_expected = tmp_const_exp ;
			if (value_result == nullptr) 
            {
                error_flag = 1 ;
                err.error(node.array_index->line, node.array_index->pos, "Index must be a constant!") ;
				return;
			}
            if (value_result->getType() != TYPE32)  // 索引为非整型
            {
                error_flag = 1 ;
                err.error(node.line, node.pos, "Index Invalid!") ;
                return ;
            }
			lval_as_rval = tmp_lval;
			constexpr_expected = tmp_const_exp;
			Value* indexList[] = { CONST(0), value_result };
            // Value* indexListFloat[] = { ConstantFP::get(Type::getDoubleTy(context), 0), value_result };
			auto id_ptr = std::get<0>(lookup_variable(node.name));
            value_result = builder.CreateGEP(id_ptr, indexList);
		}
	}
	else if(constexpr_expected == false){
		// 右值，需要从内存中Load
		if (node.array_index == nullptr) {      // 变量
			if (std::get<2>(lookup_variable(node.name)) == true) {
				err.error(node.line, node.pos, "Array without index : '" + node.name + "'.");
				error_flag = true;
				value_result = nullptr;
				return;
			}
			auto id_ptr = std::get<0>(lookup_variable(node.name));
			value_result = builder.CreateLoad(id_ptr);
		}
		else {
            // 检查是否是数组变量
			if (std::get<2>(lookup_variable(node.name)) == false) {
				// not array.
				err.error(node.line, node.pos, "Subscripted value is not an array: '" + node.name + "'.");
				error_flag = true;
				value_result = nullptr;
				return;
			}
			node.array_index->accept(*this);
            if (value_result->getType() != TYPE32)  // 索引为非整型
            {
                error_flag = 1 ;
                err.error(node.line, node.pos, "Index Invalid!") ;
                return ;
            }
			Value* indexList[] = {CONST(0),  value_result};
			auto id_ptr = std::get<0>(lookup_variable(node.name));
			value_result = builder.CreateGEP(id_ptr, indexList);
			value_result = builder.CreateLoad(value_result);
		}
	}
	else {
		err.error(node.line, node.pos, "Constant expression expected: '" + node.name + "'.");
		value_result = nullptr;
		if (is_result_int)
            int_const_result = 0 ;
        else
            float_const_result = 0 ;
		error_flag = true;
	}
}

void assembly_builder::visit(literal_syntax &node)
{
    // std::cout << ";literal_syntax" << std::endl ;
    if (constexpr_expected == true)
    {
        if (node.is_int)
            int_const_result = node.intConst ;
        else
            float_const_result = node.floatConst ;
    }
    else
    {
        if (node.is_int)
            value_result = CONST(node.intConst) ;
        else
            value_result = ConstantFP::get(Type::getDoubleTy(context), node.floatConst) ;
    }
    if (node.is_int)
        is_result_int = true ;
    else
    {
        is_result_int = false ;
    }
    return ;
}

void assembly_builder::visit(var_def_stmt_syntax &node)
{
    // std::cout << ";var_def_stmt_syntax" << std::endl ;
    if (variables[0].count(node.name)) {
		err.error(node.line, node.pos, "Variable " + node.name + " declared!");
		error_flag = true;
		return;
	}

	Value *valueaddress ;
    auto valuetype = node.is_int ? TYPE32 : TYPEFLOAT ;
	if (in_global == false) {
		// local
		if (node.array_length == nullptr) { 
			// 局部变量
			constexpr_expected = false;
			valueaddress = builder.CreateAlloca(valuetype, nullptr, node.name);
            // if (valuetype == TYPE32)
            //     std::cout << node.name << "yes" << std::endl ;
			// if (valueaddress->getType()->isIntegerTy(32))
            //     std::cout << "is int" << std::endl ;
            if (node.initializers.size() != 0)         // 有初始化
            {
				node.initializers[0]->accept(*this);
				if (value_result == nullptr) 
                {
					return;
				}
                if (value_result->getType() != valuetype)     // 类型转换
                {
                    // std::cout<< "test var_def" << std::endl ;
                    if (valuetype == TYPE32)
                        value_result = builder.CreateFPToSI(value_result, TYPE32) ;
                    else
                        value_result = builder.CreateSIToFP(value_result, TYPEFLOAT) ;    
                }
				builder.CreateStore(value_result, valueaddress);
			}
		}
		else 
        {
			// 局部数组变量
			constexpr_expected = true;
			node.array_length->accept(*this);
            if (is_result_int = false)
            {
                int_const_result = (int)float_const_result ;
                is_result_int = true ;
            }
			value_result = CONST(int_const_result);     //申请的数组空间大小
            int len = int_const_result ;
			constexpr_expected = false;
			// check if too much init.
			if (int_const_result < node.initializers.size()) {
				err.error(node.line, node.pos, "Don't have so much space!");
				error_flag = true;
                return ;
			}
			// check if negative length.
			if (int_const_result <= 0) {
				err.error(node.line, node.pos, "Index invalid!");
				error_flag = true;
                return ;
			}

			valueaddress = builder.CreateAlloca(ArrayType::get(valuetype, int_const_result), CONST(1), node.name);
            int i = 0;
			for (auto &init : node.initializers) {
				Value* indexList[] = { CONST(0), CONST(i++) };
				auto index = builder.CreateGEP(valueaddress, indexList) ;
				init->accept(*this);
                if (value_result->getType() != valuetype)     // 类型转换
                {
                    if (valuetype->isIntegerTy(32))
                        value_result = builder.CreateFPToSI(value_result, TYPE32) ;
                    else
                        value_result = builder.CreateSIToFP(value_result, TYPEFLOAT) ;    
                }
				builder.CreateStore(value_result, index);
			}
            while (i < len) 
            {
				Value* indexList[] = { CONST(0), CONST(i++) };
				auto index = builder.CreateGEP(valueaddress, indexList) ;
                if (valuetype->isIntegerTy(32))
                    value_result = CONST(0) ;
                else
                    value_result = ConstantFP::get(Type::getDoubleTy(context), 0) ;    
				builder.CreateStore(value_result, index);
            }
		}
	}
	else {
		// 全局变量
		constexpr_expected = true;
		if (node.array_length == nullptr) 
        {
			// 变量
            Constant *init ;
			if (node.initializers.size() == 0) 
            {
                if (valuetype == TYPE32)
                    init = CONST(0) ;
                else
                    init = ConstantFP::get(Type::getDoubleTy(context), 0) ;
			}
			else 
            {      // 需要初始化
				node.initializers[0]->accept(*this);
                if (valuetype == TYPE32)
                {
                    if (is_result_int == true)
				        init = CONST(int_const_result) ;
                    else
                        init = CONST((int)float_const_result) ;
                }
                else
                {
                    if (is_result_int == false)
                        init = ConstantFP::get(Type::getDoubleTy(context), float_const_result) ;
                    else
                        init = ConstantFP::get(Type::getDoubleTy(context), (float)int_const_result) ;
                    
                }
            }
			valueaddress = new GlobalVariable(*module.get(), valuetype, node.is_constant, GlobalValue::LinkageTypes::ExternalLinkage, init, node.name);
		}
		else 
        {
			// 全局数组
			lval_as_rval = true;
			node.array_length->accept(*this);
            int len ;
            if (is_result_int)
			    len = int_const_result;
            else
                len = (int)float_const_result ;
            
			if (len < node.initializers.size()) 
            {
				err.error(node.line, node.pos, "Don't have enough space!");
				error_flag = true;
                return ;
			}
			if (len <= 0)
            {
				err.error(node.line, node.pos, "Index Length Invalid!");
				error_flag = true;
                return ;
			}

			std::vector<Constant*> values;
			int i = 0;
			for (auto &init : node.initializers) {
				lval_as_rval = true;
				constexpr_expected = true;
                if (valuetype == TYPE32)
                    value_result = CONST(0) ;
				else
                    value_result = ConstantFP::get(Type::getDoubleTy(context), 0) ;
				init->accept(*this);
                if (valuetype == TYPE32)
                {
                    if (is_result_int)
				        values.push_back(CONST(int_const_result));
                    else
                        values.push_back(CONST((int)float_const_result));
                }
                else
                {
                    if (is_result_int == false)
                        values.push_back(ConstantFP::get(Type::getDoubleTy(context), float_const_result));
                    else
                        values.push_back(ConstantFP::get(Type::getDoubleTy(context), (float)int_const_result));
                }
            }
			for (int i = values.size(); i < len; i++) {
				if (valuetype == TYPE32)
				    values.push_back(CONST(0));
                else
                    values.push_back(ConstantFP::get(Type::getDoubleTy(context), 0));
			}
			auto init = ConstantArray::get(ArrayType::get(valuetype, len), values);

			valueaddress = new GlobalVariable(*module.get(), ArrayType::get(valuetype, len), node.is_constant, GlobalValue::LinkageTypes::ExternalLinkage, init, node.name);
		}
	}
	declare_variable(node.name, valueaddress, node.is_constant, node.array_length != nullptr, node.is_int);
}

void assembly_builder::visit(assign_stmt_syntax &node)
{
    // std::cout << ";assign_stmt_syntax" << std::endl ;
    if (std::get<0>(lookup_variable(node.target->name)) == nullptr)
    {
        error_flag = true ;
        err.error(node.target->line, node.target->pos, "Variable "+node.target->name+" not declared!") ;
        return ;
    }
    if (std::get<1>(lookup_variable(node.target->name)) == true)   // const
    {
        error_flag = true ;
        err.error(node.target->line, node.target->pos, "Variable "+node.target->name+" is a const and can't be assigned!") ;
        return ;
    }
    if (std::get<2>(lookup_variable(node.target->name)) == false && node.target->array_index ) {
		error_flag = true;
		err.error(node.target->line, node.target->pos, "Variable "+node.target->name+" isn't an array!") ;
		return;
	}
    if (std::get<2>(lookup_variable(node.target->name)) == true && node.target->array_index == nullptr) {
		error_flag = true;
        err.error(node.target->line, node.target->pos, "Variable " + node.target->name + " can't be assigned with no index");
		return;
	}
    lval_as_rval = false;
	node.target->accept(*this);
	lval_as_rval = true;
	Value *valuename = value_result;
    Value *temp = builder.CreateLoad(valuename) ;   // 用于比较类型
	node.value->accept(*this);
    Value *valuenum = value_result ;
    if (temp->getType() != valuenum->getType())
    {
        // std::cout << "test2" << std::endl ;
        if (temp->getType()->isIntegerTy(32))
            valuenum = builder.CreateFPToSI(valuenum, TYPE32) ;
        if (temp->getType()->isDoubleTy())
            valuenum = builder.CreateSIToFP(valuenum, TYPEFLOAT) ;
    }
    builder.CreateStore(valuenum, valuename) ;
}

void assembly_builder::visit(func_call_stmt_syntax &node)
{
    // std::cout << ";func_call_stmt_syntax" << std::endl ;
    auto func = functions[node.name] ;
    if (func == nullptr)
    {
        error_flag = 1 ;
        err.error(node.line, node.pos, "Can't find the function" + node.name + "\n") ;
        return ;
    }
    auto call = builder.CreateCall(func, {}) ;
    ret = call ;
    return ;
}

void assembly_builder::visit(block_syntax &node)
{
    // std::cout << ";block_syntax" << std::endl ;
    enter_scope() ;
    lval_as_rval = true;
    constexpr_expected = false;
    for (auto item : node.body)
        item->accept(*this) ;
    exit_scope() ;
    return ;
}

void assembly_builder::visit(if_stmt_syntax &node)
{
    // std::cout << ";if_stmt_syntax" << std::endl ;
    auto ifbb = BasicBlock::Create(context, "", current_function) ;
    auto elsebb = BasicBlock::Create(context, "", current_function) ;
    auto thenbb = BasicBlock::Create(context, "", current_function) ;
    auto endbranchbb = BasicBlock::Create(context, "", current_function) ;
    // ifbb
    builder.CreateBr(ifbb) ;
    builder.SetInsertPoint(ifbb) ;
    node.pred->accept(*this) ;
    builder.CreateCondBr(value_result, thenbb, elsebb) ;
    //thenbb
    builder.SetInsertPoint(thenbb) ;
    node.then_body->accept(*this) ;
    builder.CreateBr(endbranchbb) ;
    //elsebb
    builder.SetInsertPoint(elsebb) ;
    if (node.else_body != nullptr)
        node.else_body->accept(*this) ;
    builder.CreateBr(endbranchbb) ;
    builder.SetInsertPoint(endbranchbb) ;
    return ;
}

void assembly_builder::visit(while_stmt_syntax &node)
{
    // std::cout << ";while_stmt_syntax" << std::endl ;
    auto whilebb = BasicBlock::Create(context, "", current_function) ;
    auto loopbb = BasicBlock::Create(context, "", current_function) ;
    auto endwhilebb = BasicBlock::Create(context, "", current_function) ;
    //whilebb
    builder.CreateBr(whilebb) ;
    builder.SetInsertPoint(whilebb) ;
    node.pred->accept(*this) ;
    builder.CreateCondBr(value_result, loopbb, endwhilebb) ;
    builder.SetInsertPoint(loopbb) ;
    node.body->accept(*this) ;
    builder.CreateBr(whilebb) ;
    builder.SetInsertPoint(endwhilebb) ;
    return ;
}

void assembly_builder::visit(empty_stmt_syntax &node)
{
    // std::cout << ";empty_stmt_syntax" << std::endl ;
    ;
}