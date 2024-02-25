#include "tao/pegtl/rules.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>

#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/analyze.hpp>

#include <LA.h>
#include <helper.h>
#include <parser.h>

using namespace TAO_PEGTL_NAMESPACE;

namespace LA {

// clang-format off
class ItemStack {
public:
  ItemStack() : items{std::vector<Item *>()} { return; }

  void push(Item *item) { items.push_back(item); }

  Item *pop() {
    auto item = items.back();
    items.pop_back();
    return item;
  }

  Item *top() const { return items.back(); }

private:
  std::vector<Item *> items;
};

/*
 * Tokens parsed
 */
ItemStack itemStack;

/*
 * Grammar rules from now on.
 */
struct name : seq<plus<sor<alpha, one<'_'>>>, star<sor<alpha, one<'_'>, digit>>> {};

/*
 * Control characters.
 */
struct comment : disable<TAO_PEGTL_STRING("//"), until<eolf>> {};

// spaces ::= \s*
struct spaces : star<sor<one<' '>, one<'\t'>>> {};

// seps::= (\s*eol)*
struct seps : star<seq<spaces, eol>> {};

// seps_with_comments::= (\s*(eol|comment))*
struct seps_with_comments : star<seq<spaces, sor<eol, comment>>> {};

struct comma : seq<spaces, one<','>, spaces> {};

/*
 * Keywords.
 */
struct ret : TAO_PEGTL_STRING("return") {};
struct arrow : TAO_PEGTL_STRING("<-") {};
// ") <- this is used to fix the colorization

struct less_than : one<'<'> {};
struct less_equal : TAO_PEGTL_STRING("<=") {};
struct equal : one<'='> {};
struct greater_equal : TAO_PEGTL_STRING(">=") {};
struct greater_than : one<'>'> {};
struct left_shift : TAO_PEGTL_STRING("<<") {};
struct right_shift : TAO_PEGTL_STRING(">>") {};
struct add_op : one<'+'> {};
struct sub_op : one<'-'> {};
struct mul_op : one<'*'> {};
struct and_op : one<'&'> {};

struct call : TAO_PEGTL_STRING("call") {};

struct left_par : one<'('> {};
struct right_par : one<')'> {};
struct left_sq_bra : one<'['> {};
struct right_sq_bra : one<']'> {};
struct closed_sq_bra: TAO_PEGTL_STRING("[]") {};

struct print : TAO_PEGTL_STRING("print") {};
struct input : TAO_PEGTL_STRING("input") {};

struct br : TAO_PEGTL_STRING("br") {};

struct int64_str : TAO_PEGTL_STRING("int64") {};
struct int64_arr_str : int64_str {};
struct tuple_str : TAO_PEGTL_STRING("tuple") {};
struct code_str : TAO_PEGTL_STRING("code") {};
struct void_str : TAO_PEGTL_STRING("void") {};

struct new_str : TAO_PEGTL_STRING("new") {};
struct arr_str : TAO_PEGTL_STRING("Array") {};
struct tup_str : TAO_PEGTL_STRING("Tuple") {};

struct length_str : TAO_PEGTL_STRING("length") {};

// N ::= (+|-)?[1-9][0-9]* | 0
struct N : seq<opt<sor<one<'-'>, one<'+'>>>, plus<digit>> {};

// label ::= :name
struct lbl_name : name {};
struct label : seq<one<':'>, lbl_name> {};

/*
 * Combined rules.
 */

// op ::= + | - | * | & | << | >> | <= | < | = | >= | >
struct op : sor<add_op, sub_op, mul_op, and_op, left_shift, right_shift, less_equal, less_than, equal, greater_equal, greater_than> {};

// t ::= name | N    # right value in arithmetic instructions
struct t : sor<name, N> {};

// type ::= int64([])+ | int64 | tuple | code
struct type : sor<seq<at<seq<int64_arr_str, plus<closed_sq_bra>>>, seq<int64_arr_str, plus<closed_sq_bra>>>, 
                  seq<at<int64_str>, int64_str>,
                  seq<at<tuple_str>, tuple_str>, 
                  seq<at<code_str>, code_str>> {};

// T ::= type | void
struct T : sor<type, void_str> {};

// var_decl ::= type name
struct declared_var : name {};
struct var_decl : seq<type, spaces, declared_var> {};

struct index : seq<left_sq_bra, spaces, t, spaces, right_sq_bra> {};

// mem_loc ::= name([t])+
struct mem_name : name {};
struct mem_loc : seq<mem_name, plus<index>> {};

// args ::= t(, t)*
struct args : list<t, comma> {};
struct argument_list: seq<left_par, spaces, opt<args>, spaces, right_par> {};

// var_decls ::= type name(, type name)*
struct var_decls : list<var_decl, comma> {};
struct parameter_list : seq<left_par, spaces, opt<var_decls>, spaces, right_par> {};

/*
* Instructions.
*/
struct decl_inst : var_decl {};

struct op_inst : seq<name, spaces, arrow, spaces, t, spaces, op, spaces, t> {};

struct load_inst : seq<name, spaces, arrow, spaces, mem_loc> {};

struct store_inst : seq<mem_loc, spaces, arrow, spaces, t> {};

struct assign_inst: seq<name, spaces, arrow, spaces, t> {};

struct arr_len_inst : seq<name, spaces, arrow, spaces, length_str, spaces, name, spaces, t> {};

struct tup_len_inst : seq<name, spaces, arrow, spaces, length_str, spaces, name> {};

struct new_arr_inst : seq<name, spaces, arrow, spaces, new_str, spaces, arr_str, spaces, left_par, spaces, args, spaces, right_par> {};

struct new_tup_inst : seq<name, spaces, arrow, spaces, new_str, spaces, tup_str, spaces, left_par, spaces, t, spaces, right_par> {};

struct ret_val_inst : seq<ret, spaces, t> {};

struct ret_inst : ret {};

struct label_inst : label {};

struct branch_inst : seq<br, spaces, label> {};

struct cond_branch_inst : seq<br, spaces, t, spaces, label, spaces, label> {};

struct runtime_func_call : seq<sor<print, input>, spaces, argument_list> {};
struct user_funcs : name {};
struct user_func_call : seq<user_funcs, spaces, argument_list> {};
struct call_inst : sor<
                        seq<at<runtime_func_call>, runtime_func_call>,
                        seq<at<user_func_call>, user_func_call>
                      > {};

struct func_call : call_inst {};
struct call_assign_inst : seq<name, spaces, arrow, spaces, func_call> {};

struct i : sor<
              seq<at<decl_inst>, decl_inst>,
              seq<at<op_inst>, op_inst>,
              seq<at<load_inst>, load_inst>,
              seq<at<store_inst>, store_inst>,
              seq<at<call_inst>, call_inst>,
              seq<at<call_assign_inst>, call_assign_inst>,
              seq<at<arr_len_inst>, arr_len_inst>,
              seq<at<tup_len_inst>, tup_len_inst>,
              seq<at<new_arr_inst>, new_arr_inst>,
              seq<at<new_tup_inst>, new_tup_inst>,
              seq<at<assign_inst>, assign_inst>,
              seq<at<comment>, comment>,
              seq<at<branch_inst>, branch_inst>,
              seq<at<cond_branch_inst>, cond_branch_inst>,
              seq<at<ret_val_inst>, ret_val_inst>,
              seq<at<ret_inst>, ret_inst>,
              seq<at<label_inst>, label_inst>
            > {};

struct instructions : star<seq<seps, bol, spaces, i, seps>> {};

struct func_def_name : name {};

struct function : seq<
                    seq<spaces, T>,
                    seps_with_comments,
                    seq<spaces, func_def_name>,
                    seps_with_comments,
                    seq<spaces, parameter_list>,
                    seps_with_comments,
                    seq<spaces, one<'{'>>,
                    seps_with_comments,
                    instructions,
                    seps_with_comments,
                    seq<spaces, one<'}'>>
                  > {};

struct entry_point : plus<seps_with_comments, function, seps_with_comments> {};

struct grammar : must<entry_point> {};
// clang-format on

/*
 * Actions attached to grammar rules.
 */
template <typename Rule> struct action : nothing<Rule> {};

template <> struct action<less_than> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto op = Operator::getOperator(Operator::ID::LESS_THAN);
    itemStack.push(op);
    debug("parsed less than operator");
  }
};

template <> struct action<less_equal> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto op = Operator::getOperator(Operator::ID::LESS_EQUAL);
    itemStack.push(op);
    debug("parsed less equal operator");
  }
};

template <> struct action<equal> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto op = Operator::getOperator(Operator::ID::EQUAL);
    itemStack.push(op);
    debug("parsed equal operator");
  }
};

template <> struct action<greater_equal> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto op = Operator::getOperator(Operator::ID::GREATER_EQUAL);
    itemStack.push(op);
    debug("parsed greater equal operator");
  }
};

template <> struct action<greater_than> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto op = Operator::getOperator(Operator::ID::GREATER_THAN);
    itemStack.push(op);
    debug("parsed greater than operator");
  }
};

template <> struct action<left_shift> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto op = Operator::getOperator(Operator::ID::LS);
    itemStack.push(op);
    debug("parsed left shift operator");
  }
};

template <> struct action<right_shift> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto op = Operator::getOperator(Operator::ID::RS);
    itemStack.push(op);
    debug("parsed right shift operator");
  }
};

template <> struct action<add_op> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto op = Operator::getOperator(Operator::ID::ADD);
    itemStack.push(op);
    debug("parsed add operator");
  }
};

template <> struct action<sub_op> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto op = Operator::getOperator(Operator::ID::SUB);
    itemStack.push(op);
    debug("parsed sub operator");
  }
};

template <> struct action<mul_op> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto op = Operator::getOperator(Operator::ID::MUL);
    itemStack.push(op);
    debug("parsed multiplication operator");
  }
};

template <> struct action<and_op> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto op = Operator::getOperator(Operator::ID::AND);
    itemStack.push(op);
    debug("parsed and operator");
  }
};

template <> struct action<left_par> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    itemStack.push(LeftParen::getInstance());
    debug("parsed left parenthesis");
  }
};

template <> struct action<right_par> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    itemStack.push(RightParen::getInstance());
    debug("parsed right parenthesis");
  }
};

template <> struct action<closed_sq_bra> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto arrType = (ArrayType *)itemStack.top();
    arrType->increaseDim();
    debug("parsed closed square bracket");
  }
};

template <> struct action<print> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto callee = RuntimeFunction::getRuntimeFunction(RuntimeFunction::ID::PRINT);
    itemStack.push(callee);
    debug("parsed runtime function " + callee->toStr());
  }
};

template <> struct action<input> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto callee = RuntimeFunction::getRuntimeFunction(RuntimeFunction::ID::INPUT);
    itemStack.push(callee);
    debug("parsed runtime function " + callee->toStr());
  }
};

template <> struct action<int64_arr_str> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto arrType = new ArrayType();
    itemStack.push(arrType);
    debug("parsed int64 array type");
  }
};

template <> struct action<int64_str> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    itemStack.push(Int64Type::getInstance());
    debug("parsed int64 type");
  }
};

template <> struct action<tuple_str> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    itemStack.push(TupleType::getInstance());
    debug("parsed tuple type");
  }
};

template <> struct action<code_str> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    itemStack.push(CodeType::getInstance());
    debug("parsed code type");
  }
};

template <> struct action<void_str> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    itemStack.push(VoidType::getInstance());
    debug("parsed void type");
  }
};

template <> struct action<T> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto F = new Function();
    auto type = (VarType *)itemStack.pop();
    F->setReturnType(type);
    P.addFunction(F);
    debug("parsed function return type " + type->toStr());
  }
};

template <> struct action<N> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto n = new Number(std::stoll(in.string()));
    itemStack.push(n);
    debug("parsed number " + n->toStr());
  }
};

template <> struct action<label> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto l = P.getLabelInCurrFunc(in.string());
    itemStack.push(l);
    debug("parsed label " + l->getName());
  }
};

template <> struct action<user_funcs> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto name = in.string();
    Name *callee;

    if (P.currFuncHasVariable(name))
      callee = P.getVariableInCurrFunc(name);
    else
      callee = new UserFunction(name);

    itemStack.push(callee);
    debug("parsed function name " + callee->toStr());
  }
};

template <> struct action<func_def_name> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto F = P.getCurrFunction();
    F->setName(in.string());
    debug("parsed function name " + F->getName());
  }
};

template <> struct action<name> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto F = P.getCurrFunction();
    Name *name;
    auto nameStr = in.string();
    if (P.currFuncHasVariable(nameStr))
      name = P.getVariableInCurrFunc(nameStr);
    else
      name = new UserFunction(nameStr);
    itemStack.push(name);
    debug("parsed name " + name->toStr());
  }
};

template <> struct action<declared_var> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto name = in.string();
    auto type = (const VarType *)itemStack.pop();
    P.declareVariableInCurrFunc(name, type);
    itemStack.push(P.getVariableInCurrFunc(name));
    debug("parsed declared variable " + type->toStr() + " " + name);
  }
};

template <> struct action<mem_name> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto base = P.getVariableInCurrFunc(in.string());
    auto memoryLocation = new MemoryLocation(base);
    itemStack.push(memoryLocation);
    debug("parsed memory variable " + memoryLocation->toStr());
  }
};

template <> struct action<index> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto index = (Value *)itemStack.pop();
    auto memLoc = (MemoryLocation *)itemStack.top();
    memLoc->addIndex(index);
    debug("parsed array index " + memLoc->toStr());
  }
};

template <> struct action<argument_list> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto curr = itemStack.pop();
    if (!dynamic_cast<const RightParen *>(curr))
      throw std::runtime_error("Expected right parenthesis");

    auto args = new Arguments();
    while (true) {
      curr = itemStack.pop();
      if (curr == LeftParen::getInstance())
        break;
      args->addArgToHead((const Value *)curr);
    }
    itemStack.push(args);
    debug("parsed argument list (" + args->toStr() + ")");
  }
};

template <> struct action<parameter_list> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto curr = itemStack.pop();
    if (!dynamic_cast<const RightParen *>(curr))
      throw std::runtime_error("Expected right parenthesis");

    auto params = new Parameters();
    while (true) {
      curr = itemStack.pop();
      if (curr == LeftParen::getInstance())
        break;

      params->addParamToHead((Variable *)curr);
    }
    auto F = P.getCurrFunction();
    F->setParams(params);
    debug("parsed parameter list (" + params->toStr() + ")");
  }
};

template <> struct action<decl_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto var = (Variable *)itemStack.pop();
    auto I = new DeclarationInst(var);
    P.addInstructionToCurrFunc(I, in.position().line);
    debug("parsed declaration instruction " + I->toStr());
  }
};

template <> struct action<assign_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto rval = (Value *)itemStack.pop();
    auto lval = (Variable *)itemStack.pop();
    auto I = new AssignInst(lval, rval);
    P.addInstructionToCurrFunc(I, in.position().line);
    debug("parsed assignment instruction " + I->toStr());
  }
};

template <> struct action<op_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto arithRval = (const Value *)itemStack.pop();
    auto op = (const Operator *)itemStack.pop();
    auto arithLval = (const Value *)itemStack.pop();
    auto lval = (Variable *)itemStack.pop();
    auto I = new OpInst(lval, arithLval, op, arithRval);
    P.addInstructionToCurrFunc(I, in.position().line);
    debug("parsed arithmetic instruction " + I->toStr());
  }
};

template <> struct action<load_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto memLoc = (const MemoryLocation *)itemStack.pop();
    auto target = (Variable *)itemStack.pop();
    auto I = new LoadInst(target, memLoc);
    P.addInstructionToCurrFunc(I, in.position().line);
    debug("parsed load instruction " + I->toStr());
  }
};

template <> struct action<store_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto source = (const Value *)itemStack.pop();
    auto memLoc = (const MemoryLocation *)itemStack.pop();
    auto I = new StoreInst(memLoc, source);
    P.addInstructionToCurrFunc(I, in.position().line);
    debug("parsed store instruction " + I->toStr());
  }
};

template <> struct action<arr_len_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto dimIndex = (const Value *)itemStack.pop();
    auto base = (Variable *)itemStack.pop();
    auto result = (Variable *)itemStack.pop();
    auto I = new ArrayLenInst(result, base, dimIndex);
    P.addInstructionToCurrFunc(I, in.position().line);
    debug("parsed array length instruction " + I->toStr());
  }
};

template <> struct action<tup_len_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto base = (Variable *)itemStack.pop();
    auto result = (Variable *)itemStack.pop();
    auto I = new TupleLenInst(result, base);
    P.addInstructionToCurrFunc(I, in.position().line);
    debug("parsed tuple length instruction " + I->toStr());
  }
};

template <> struct action<new_arr_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto curr = itemStack.pop();
    if (!dynamic_cast<RightParen *>(curr))
      throw std::runtime_error("Expected right parenthesis");

    std::vector<const Value *> sizes;
    while (true) {
      curr = itemStack.pop();
      if (curr == LeftParen::getInstance())
        break;
      sizes.insert(sizes.begin(), (Value *)curr);
    }

    auto array = (Variable *)itemStack.pop();
    auto I = new NewArrayInst(array, sizes);
    P.addInstructionToCurrFunc(I, in.position().line);
    debug("parsed new array instruction " + I->toStr());
  }
};

template <> struct action<new_tup_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    itemStack.pop();
    auto size = (const Value *)itemStack.pop();
    itemStack.pop();
    auto tuple = (Variable *)itemStack.pop();
    auto I = new NewTupleInst(tuple, size);
    P.addInstructionToCurrFunc(I, in.position().line);
    debug("parsed new tuple instruction " + I->toStr());
  }
};

template <> struct action<ret_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto I = new RetInst();
    auto currBB = P.getCurrFunction()->getCurrBasicBlock();
    currBB->setTerminator(I);
    P.newBasicBlock();
    debug("parsed return instruction " + I->toStr());
  }
};

template <> struct action<ret_val_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    debug("parsed return value instruction");
    auto rval = (const Value *)itemStack.pop();
    auto I = new RetValueInst(rval);
    auto currBB = P.getCurrFunction()->getCurrBasicBlock();
    currBB->setTerminator(I);
    P.newBasicBlock();
    debug("parsed return value instruction " + I->toStr());
  }
};

template <> struct action<label_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto label = P.getLabelInCurrFunc(in.string());
    auto currBB = P.newBasicBlock();
    currBB->setLabel(label);
    debug("parsed label instruction " + label->toStr());
  }
};

template <> struct action<branch_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto label = (const Label *)itemStack.pop();
    auto I = new BranchInst(label);
    auto currBB = P.getCurrFunction()->getCurrBasicBlock();
    currBB->setTerminator(I);
    P.newBasicBlock();
    debug("parsed branch instruction " + I->toStr());
  }
};

template <> struct action<cond_branch_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto falseLabel = (const Label *)itemStack.pop();
    auto trueLabel = (const Label *)itemStack.pop();
    auto condition = (const Value *)itemStack.pop();
    auto I = new CondBranchInst(condition, trueLabel, falseLabel);
    auto currBB = P.getCurrFunction()->getCurrBasicBlock();
    currBB->setTerminator(I);
    P.newBasicBlock();
    debug("parsed conditional branch instruction " + I->toStr());
  }
};

template <> struct action<call_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto args = (const Arguments *)itemStack.pop();
    auto callee = (Name *)itemStack.pop();
    auto I = new CallInst(callee, args);
    P.addInstructionToCurrFunc(I, in.position().line);

    debug("parsed call instruction " + I->toStr());
  }
};

template <> struct action<call_assign_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto args = (const Arguments *)itemStack.pop();
    auto callee = (Name *)itemStack.pop();
    auto lval = (Variable *)itemStack.pop();
    auto I = new CallAssignInst(lval, callee, args);
    P.addInstructionToCurrFunc(I, in.position().line);

    debug("parsed call assignment instruction " + I->toStr());
  }
};

Program *parseFile(char *fileName) {

  /*
   * Check the grammar for some possible issues.
   */
  if (analyze<grammar>() != 0) {
    std::cerr << "There are problems with the grammar" << std::endl;
    exit(1);
  }

  /*
   * Parse.
   */
  file_input<> fileInput(fileName);
  auto P = new Program();
  parse<grammar, action>(fileInput, *P);
  return P;
}

} // namespace LA
