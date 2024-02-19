#include <iostream>
#include <stdexcept>
#include <vector>

#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/analyze.hpp>

#include <IR.h>
#include <helper.h>
#include <parser.h>

using namespace TAO_PEGTL_NAMESPACE;

namespace IR {

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

struct def : TAO_PEGTL_STRING("define") {};

struct less_than : one<'<'> {};
struct less_equal : TAO_PEGTL_STRING("<=") {};
struct equal : one<'='> {};
struct greater_equal : TAO_PEGTL_STRING(">=") {};
struct greater_than : one<'>'> {};

struct ls_op : TAO_PEGTL_STRING("<<") {};
struct rs_op : TAO_PEGTL_STRING(">>") {};
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
struct tuple_error : TAO_PEGTL_STRING("tuple-error") {};
struct tensor_error : TAO_PEGTL_STRING("tensor-error") {};

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

// label ::= name
struct label : seq<one<':'>, name> {};

// I ::= @name    # function names
struct I : seq<one<'@'>, name> {};
struct func : I {};

// var ::= %name
struct var : seq<one<'%'>, name> {};
struct dec_var : var {};
struct mem_var : var {};

/*
 * Combined rules.
 */
// cmp ::= <= | < | = | >= | >
struct cmp : sor<less_equal, less_than, equal, greater_equal, greater_than> {};

// op ::= + | - | * | & | << | >>
struct op : sor<add_op, sub_op, mul_op, and_op, ls_op, rs_op> {};

// u ::= var | I
struct u : sor<var, I> {};

// t ::= var | N    # right value in arithmetic instructions
struct t : sor<var, N> {};

// s ::= t | label | I    # can be used as right value in assignment instructions
struct s : sor<t, I> {};

// callee ::= u | print | input | tuple-error | tensor-error
struct callee : sor<u, print, input, tuple_error, tensor_error> {};

// type ::= int64([])+ | int64 | tuple | code
struct type : sor<seq<at<seq<int64_arr_str, plus<closed_sq_bra>>>, seq<int64_arr_str, plus<closed_sq_bra>>>, 
                  seq<at<int64_str>, int64_str>,
                  seq<at<tuple_str>, tuple_str>, 
                  seq<at<code_str>, code_str>> {};

// T ::= type | void
struct T : sor<type, void_str> {};

// type_decl ::= type var
struct type_decl : seq<type, spaces, dec_var> {};

struct arr_index : seq<left_sq_bra, spaces, t, spaces, right_sq_bra> {};

// mem_loc ::= var([t])+
struct mem_loc : seq<mem_var, plus<arr_index>> {};

// args ::= t(, t)*
struct args : list<t, comma> {};
struct argument_list: seq<left_par, spaces, opt<args>, spaces, right_par> {};

// vars ::= type var(, type var)*
struct vars : list<type_decl, comma> {};
struct parameter_list : seq<left_par, spaces, opt<vars>, spaces, right_par> {};

/*
* Instructions.
*/
struct decl_inst : type_decl {};

struct comp_inst : seq<var, spaces, arrow, spaces, t, spaces, cmp, spaces, t> {};

struct arith_inst : seq<var, spaces, arrow, spaces, t, spaces, op, spaces, t> {};

struct load_inst : seq<var, spaces, arrow, spaces, mem_loc> {};

struct store_inst : seq<mem_loc, spaces, arrow, spaces, s> {};

struct assign_inst: seq<var, spaces, arrow, spaces, s> {};

struct arr_len_inst : seq<var, spaces, arrow, spaces, length_str, spaces, var, spaces, t> {};

struct tup_len_inst : seq<var, spaces, arrow, spaces, length_str, spaces, var> {};

struct new_arr_inst : seq<var, spaces, arrow, spaces, new_str, spaces, arr_str, spaces, left_par, spaces, args, spaces, right_par> {};

struct new_tup_inst : seq<var, spaces, arrow, spaces, new_str, spaces, tup_str, spaces, left_par, spaces, t, spaces, right_par> {};

struct ret_val_inst : seq<ret, spaces, t> {};

struct ret_inst : ret {};

struct label_inst : label {};

struct branch_inst : seq<br, spaces, label> {};

struct cond_branch_inst : seq<br, spaces, t, spaces, label, spaces, label> {};

struct call_inst : seq<call, spaces, callee, spaces, argument_list> {};

struct call_assign_inst : seq<var, spaces, arrow, spaces, call, spaces, callee, spaces, argument_list> {};

struct i : sor<
              seq<at<decl_inst>, decl_inst>,
              seq<at<comp_inst>, comp_inst>,
              seq<at<arith_inst>, arith_inst>,
              seq<at<load_inst>, load_inst>,
              seq<at<store_inst>, store_inst>,
              seq<at<assign_inst>, assign_inst>,
              seq<at<arr_len_inst>, arr_len_inst>,
              seq<at<tup_len_inst>, tup_len_inst>,
              seq<at<new_arr_inst>, new_arr_inst>,
              seq<at<new_tup_inst>, new_tup_inst>,
              seq<at<call_inst>, call_inst>,
              seq<at<call_assign_inst>, call_assign_inst>,
              seq<at<comment>, comment>
            > {};

struct te : sor<
              seq<at<branch_inst>, branch_inst>,
              seq<at<cond_branch_inst>, cond_branch_inst>,
              seq<at<ret_val_inst>, ret_val_inst>,
              seq<at<ret_inst>, ret_inst>
            > {};

struct instructions : star<seq<seps, bol, spaces, i, seps>> {};

struct bb : seq<
              bol, spaces, label_inst, 
              seps_with_comments,
              instructions,
              seps_with_comments,
              bol, spaces, te
            > {};

struct bbs : plus<seq<seps_with_comments, bb, seps_with_comments>> {};

struct function : seq<
                    seq<spaces, def>,
                    seps_with_comments,
                    seq<spaces, T>,
                    seps_with_comments,
                    seq<spaces, func>,
                    seps_with_comments,
                    seq<spaces, parameter_list>,
                    seps_with_comments,
                    seq<spaces, one<'{'>>,
                    seps_with_comments,
                    bbs,
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

template <> struct action<def> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto F = new Function();
    P.addFunction(F);
    debug("parsed function definition");
  }
};

template <> struct action<less_than> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto op = CompareOp::getCompareOp(CompareOp::ID::LESS_THAN);
    itemStack.push(op);
    debug("parsed less than operator");
  }
};

template <> struct action<less_equal> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto op = CompareOp::getCompareOp(CompareOp::ID::LESS_EQUAL);
    itemStack.push(op);
    debug("parsed less equal operator");
  }
};

template <> struct action<equal> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto op = CompareOp::getCompareOp(CompareOp::ID::EQUAL);
    itemStack.push(op);
    debug("parsed equal operator");
  }
};

template <> struct action<greater_equal> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto op = CompareOp::getCompareOp(CompareOp::ID::GREATER_EQUAL);
    itemStack.push(op);
    debug("parsed greater equal operator");
  }
};

template <> struct action<greater_than> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto op = CompareOp::getCompareOp(CompareOp::ID::GREATER_THAN);
    itemStack.push(op);
    debug("parsed greater than operator");
  }
};

template <> struct action<ls_op> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto op = ArithOp::getArithOp(ArithOp::ID::LS);
    itemStack.push(op);
    debug("parsed left shift operator");
  }
};

template <> struct action<rs_op> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto op = ArithOp::getArithOp(ArithOp::ID::RS);
    itemStack.push(op);
    debug("parsed right shift operator");
  }
};

template <> struct action<add_op> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto op = ArithOp::getArithOp(ArithOp::ID::ADD);
    itemStack.push(op);
    debug("parsed add operator");
  }
};

template <> struct action<sub_op> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto op = ArithOp::getArithOp(ArithOp::ID::SUB);
    itemStack.push(op);
    debug("parsed sub operator");
  }
};

template <> struct action<mul_op> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto op = ArithOp::getArithOp(ArithOp::ID::MUL);
    itemStack.push(op);
    debug("parsed multiplication operator");
  }
};

template <> struct action<and_op> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto op = ArithOp::getArithOp(ArithOp::ID::AND);
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

template <> struct action<tuple_error> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto callee = RuntimeFunction::getRuntimeFunction(RuntimeFunction::ID::TUPLE_ERROR);
    itemStack.push(callee);
    debug("parsed runtime function " + callee->toStr());
  }
};

template <> struct action<tensor_error> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto callee = RuntimeFunction::getRuntimeFunction(RuntimeFunction::ID::TENSOR_ERROR);
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
    auto tupleType = new TupleType();
    itemStack.push(tupleType);
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
    auto F = P.getCurrFunction();
    auto type = (Type *)itemStack.pop();
    F->setReturnType(type);
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
    auto l = P.getLabel(in.string());
    itemStack.push(l);
    debug("parsed label " + l->getName());
  }
};

template <> struct action<I> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto n = new FunctionName(in.string());
    itemStack.push(n);
    debug("parsed function name " + n->getName());
  }
};

template <> struct action<func> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto F = P.getCurrFunction();
    F->setName(in.string());
    debug("parsed function name " + F->getName());
  }
};

template <> struct action<var> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto F = P.getCurrFunction();
    auto var = P.getVariable(in.string());
    itemStack.push(var);
    debug("parsed variable " + var->toStr());
  }
};

template <> struct action<dec_var> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto name = in.string();
    auto type = (const Type *)itemStack.pop();
    P.defineVariable(name, type);
    itemStack.push(P.getVariable(name));
    debug("parsed variable declaration " + type->toStr() + " " + name);
  }
};

template <> struct action<mem_var> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto base = P.getVariable(in.string());
    auto memoryLocation = new MemoryLocation(base);
    itemStack.push(memoryLocation);
    debug("parsed memory variable " + memoryLocation->toStr());
  }
};

template <> struct action<arr_index> {
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

      params->addParamToHead((const Variable *)curr);
    }
    auto F = P.getCurrFunction();
    F->setParams(params);
    debug("parsed parameter list (" + params->toStr() + ")");
  }
};

template <> struct action<decl_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto var = (const Variable *)itemStack.pop();
    auto I = new DeclarationInst(var);
    P.addInstruction(I);
    debug("parsed type declaration " + I->toStr());
  }
};

template <> struct action<assign_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto rval = itemStack.pop();
    auto lval = (const Variable *)itemStack.pop();
    auto I = new AssignInst(lval, rval);
    P.addInstruction(I);
    debug("parsed assignment instruction " + I->toStr());
  }
};

template <> struct action<arith_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto arithRval = (const Value *)itemStack.pop();
    auto op = (const ArithOp *)itemStack.pop();
    auto arithLval = (const Value *)itemStack.pop();
    auto lval = (const Variable *)itemStack.pop();
    auto I = new ArithInst(lval, arithLval, op, arithRval);
    P.addInstruction(I);
    debug("parsed arithmetic instruction " + I->toStr());
  }
};

template <> struct action<comp_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto compRval = (const Value *)itemStack.pop();
    auto cmp = (const CompareOp *)itemStack.pop();
    auto compLval = (const Value *)itemStack.pop();
    auto lval = (const Variable *)itemStack.pop();
    auto I = new CompareInst(lval, compLval, cmp, compRval);
    P.addInstruction(I);
    debug("parsed comparison instruction " + I->toStr());
  }
};

template <> struct action<load_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto memLoc = (const MemoryLocation *)itemStack.pop();
    auto target = (const Variable *)itemStack.pop();
    auto I = new LoadInst(target, memLoc);
    P.addInstruction(I);
    debug("parsed load instruction " + I->toStr());
  }
};

template <> struct action<store_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto source = (const Value *)itemStack.pop();
    auto memLoc = (const MemoryLocation *)itemStack.pop();
    auto I = new StoreInst(memLoc, source);
    P.addInstruction(I);
    debug("parsed store instruction " + I->toStr());
  }
};

template <> struct action<arr_len_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto dimIndex = (const Value *)itemStack.pop();
    auto base = (const Variable *)itemStack.pop();
    auto result = (const Variable *)itemStack.pop();
    auto I = new ArrayLenInst(result, base, dimIndex);
    P.addInstruction(I);
    debug("parsed array length instruction " + I->toStr());
  }
};

template <> struct action<tup_len_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto base = (const Variable *)itemStack.pop();
    auto result = (const Variable *)itemStack.pop();
    auto I = new TupleLenInst(result, base);
    P.addInstruction(I);
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
    auto arrayType = (ArrayType *)array->getType();
    arrayType->setSizes(sizes);
    auto I = new NewArrayInst(array);
    P.addInstruction(I);
    debug("parsed new array instruction " + I->toStr());
  }
};

template <> struct action<new_tup_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    itemStack.pop();
    auto size = (const Value *)itemStack.pop();
    itemStack.pop();
    auto tuple = (const Variable *)itemStack.pop();
    auto tupleType = (TupleType *)tuple->getType();
    tupleType->setSize(size);
    auto I = new NewTupleInst(tuple);
    P.addInstruction(I);
    debug("parsed new tuple instruction " + I->toStr());
  }
};

template <> struct action<ret_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto I = new RetInst();
    P.addInstruction(I);
    debug("parsed return instruction " + I->toStr());
  }
};

template <> struct action<ret_val_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    debug("parsed return value instruction");
    auto rval = (const Value *)itemStack.pop();
    auto I = new RetValueInst(rval);
    P.addInstruction(I);
    debug("parsed return value instruction " + I->toStr());
  }
};

template <> struct action<label_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto label = P.getLabel(in.string());
    auto I = new LabelInst(label);
    P.newBasicBlock();
    P.addInstruction(I);
    debug("parsed label instruction " + I->toStr());
  }
};

template <> struct action<branch_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto label = (const Label *)itemStack.pop();
    auto I = new BranchInst(label);
    P.addInstruction(I);
    debug("parsed branch instruction " + I->toStr());
  }
};

template <> struct action<cond_branch_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto falseLabel = (const Label *)itemStack.pop();
    auto trueLabel = (const Label *)itemStack.pop();
    auto condition = (const Value *)itemStack.pop();
    auto I = new CondBranchInst(condition, trueLabel, falseLabel);
    P.addInstruction(I);
    debug("parsed conditional branch instruction " + I->toStr());
  }
};

template <> struct action<call_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto args = (const Arguments *)itemStack.pop();
    auto callee = itemStack.pop();
    auto I = new CallInst(callee, args);
    P.addInstruction(I);

    debug("parsed call instruction " + I->toStr());
  }
};

template <> struct action<call_assign_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto args = (const Arguments *)itemStack.pop();
    auto callee = itemStack.pop();
    auto lval = (const Variable *)itemStack.pop();
    auto I = new CallAssignInst(lval, callee, args);
    P.addInstruction(I);

    debug("parsed call assignment instruction " + I->toStr());
  }
};

void linkBasicBlocks(Function *F) {
  debug("Started linking basic blocks for function " + F->getName());

  std::map<std::string, BasicBlock *> labelToBB;
  // find all basic blocks that starts with a label
  // these BBs may have predecessors that are not linked yet
  for (auto &BB : F->getBasicBlocks())
    if (auto inst = dynamic_cast<const LabelInst *>(BB->getFirstInstruction()))
      labelToBB[inst->getLabel()->getName()] = BB;

  // link all basic blocks
  for (auto &BB : F->getBasicBlocks()) {
    if (auto inst = dynamic_cast<const BranchInst *>(BB->getTerminator())) {
      auto label = inst->getLabel();
      auto targetBB = labelToBB[label->getName()];
      BB->addSuccessor(targetBB);
      targetBB->addPredecessor(BB);
    } else if (auto inst = dynamic_cast<const CondBranchInst *>(BB->getTerminator())) {
      auto labels = {inst->getTrueLabel(), inst->getFalseLabel()};
      for (auto label : labels) {
        auto targetBB = labelToBB[label->getName()];
        BB->addSuccessor(targetBB);
        targetBB->addPredecessor(BB);
      }
    }
  }
}

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
  for (auto F : P->getFunctions()) {
    linkBasicBlocks(F);
  }
  return P;
}

} // namespace IR
