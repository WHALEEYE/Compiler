#include <iostream>
#include <stdexcept>
#include <vector>

#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/analyze.hpp>

#include <L3.h>
#include <helper.h>
#include <parser.h>

using namespace TAO_PEGTL_NAMESPACE;

namespace L3 {

// clang-format off
class ItemStack {
public:
  ItemStack() : items{std::vector<const Item *>()} { return; }

  void push(const Item *item) { items.push_back(item); }

  const Item *pop() {
    auto item = items.back();
    items.pop_back();
    return item;
  }

private:
  std::vector<const Item *> items;
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

struct print : TAO_PEGTL_STRING("print") {};
struct input : TAO_PEGTL_STRING("input") {};
struct tuple_error : TAO_PEGTL_STRING("tuple-error") {};
struct tensor_error : TAO_PEGTL_STRING("tensor-error") {};

struct load : TAO_PEGTL_STRING("load") {};
struct store : TAO_PEGTL_STRING("store") {};

struct br : TAO_PEGTL_STRING("br") {};

// N ::= (+|-)?[1-9][0-9]* | 0
struct N : seq<opt<sor<one<'-'>, one<'+'>>>, plus<digit>> {};

// label ::= name
struct label : seq<one<':'>, name> {};

// I ::= @name    # function names
struct I : seq<one<'@'>, name> {};
struct func : I {};

// var ::= %name
struct var : seq<one<'%'>, name> {};

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
struct s : sor<t, label, I> {};

// args ::= t (, t)*
struct args : list<t, comma> {};
struct argument_list: seq<left_par, spaces, opt<args>, spaces, right_par> {};

// vars ::= var (, var)*
struct vars : list<var, comma> {};
struct parameter_list : seq<left_par, spaces, opt<vars>, spaces, right_par> {};

// callee ::= u | print | input | allocate | tuple-error | tensor-error
struct callee : sor<u, print, allocate, input, tuple_error, tensor_error> {};

/*
* Instructions.
*/
struct comp_inst : seq<var, spaces, arrow, spaces, t, spaces, cmp, spaces, t> {};

struct assign_inst: seq<var, spaces, arrow, spaces, s> {};

struct arith_inst : seq<var, spaces, arrow, spaces, t, spaces, op, spaces, t> {};

struct load_inst : seq<var, spaces, arrow, spaces, load, spaces, var> {};

struct store_inst : seq<store, spaces, var, spaces, arrow, spaces, s> {};

struct ret_inst : ret {};

struct ret_val_inst : seq<ret, spaces, t> {};

struct label_inst : label {};

struct branch_inst : seq<br, spaces, label> {};

struct cond_branch_inst : seq<br, spaces, t, spaces, label> {};

struct call_inst : seq<call, spaces, callee, spaces, argument_list> {};

struct call_assign_inst : seq<var, spaces, arrow, spaces, call, spaces, callee, spaces, argument_list> {};

struct i : sor<
              seq<at<comp_inst>, comp_inst>,
              seq<at<arith_inst>, arith_inst>,
              seq<at<load_inst>, load_inst>,
              seq<at<store_inst>, store_inst>,
              seq<at<ret_val_inst>, ret_val_inst>,
              seq<at<ret_inst>, ret_inst>,
              seq<at<label_inst>, label_inst>,
              seq<at<branch_inst>, branch_inst>,
              seq<at<cond_branch_inst>, cond_branch_inst>,
              seq<at<call_inst>, call_inst>,
              seq<at<call_assign_inst>, call_assign_inst>,
              seq<at<comment>, comment>,
              seq<at<assign_inst>, assign_inst>
            > {};

struct instructions : plus<seq<seps, bol, spaces, i, seps>> {};

struct function : seq<
                    seq<spaces, def>,
                    seps_with_comments,
                    seq<spaces, func>,
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

template <> struct action<allocate> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto callee = RuntimeFunction::getRuntimeFunction(RuntimeFunction::ID::ALLOCATE);
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
    auto newF = new Function(in.string());
    P.addFunction(newF);
    debug("parsed function declaration " + newF->getName());
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
    auto addr = (const Variable *)itemStack.pop();
    auto lval = (const Variable *)itemStack.pop();
    auto I = new LoadInst(lval, addr);
    P.addInstruction(I);
    debug("parsed load instruction " + I->toStr());
  }
};

template <> struct action<store_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto rval = (const Value *)itemStack.pop();
    auto addr = (const Variable *)itemStack.pop();
    auto I = new StoreInst(addr, rval);
    P.addInstruction(I);
    debug("parsed store instruction " + I->toStr());
  }
};

template <> struct action<ret_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto I = new RetInst();
    P.addInstruction(I);
    P.newContext();
    P.newBasicBlock();
    debug("parsed return instruction " + I->toStr());
  }
};

template <> struct action<ret_val_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    debug("parsed return value instruction");
    auto rval = (const Value *)itemStack.pop();
    auto I = new RetValueInst(rval);
    P.addInstruction(I);
    P.newContext();
    P.newBasicBlock();
    debug("parsed return value instruction " + I->toStr());
  }
};

template <> struct action<label_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto label = P.getLabel(in.string());
    auto I = new LabelInst(label);
    P.closeContext();
    P.newBasicBlock();
    P.addInstruction(I);
    P.newContext();
    debug("parsed label instruction " + I->toStr());
  }
};

template <> struct action<branch_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto label = (const Label *)itemStack.pop();
    auto I = new BranchInst(label);
    P.addInstruction(I);
    P.newContext();
    P.newBasicBlock();
    debug("parsed branch instruction " + I->toStr());
  }
};

template <> struct action<cond_branch_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto label = (const Label *)itemStack.pop();
    auto condition = (const Value *)itemStack.pop();
    auto I = new CondBranchInst(condition, label);
    P.addInstruction(I);
    P.newContext();
    P.newLinkedBasicBlock();
    debug("parsed conditional branch instruction " + I->toStr());
  }
};

template <> struct action<call_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto args = (const Arguments *)itemStack.pop();
    auto callee = itemStack.pop();
    auto I = new CallInst(callee, args);
    P.closeContext();
    P.addInstruction(I);
    P.newContext();
    debug("parsed call instruction " + I->toStr());
  }
};

template <> struct action<call_assign_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto args = (const Arguments *)itemStack.pop();
    auto callee = itemStack.pop();
    auto lval = (const Variable *)itemStack.pop();
    auto I = new CallAssignInst(lval, callee, args);
    P.closeContext();
    P.addInstruction(I);
    P.newContext();
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

} // namespace L3
