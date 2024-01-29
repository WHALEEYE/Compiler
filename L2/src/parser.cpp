#include "spiller.h"
#include <iostream>
#include <vector>

#include <tao/pegtl.hpp>
#include <tao/pegtl/ascii.hpp>
#include <tao/pegtl/contrib/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>
#include <tao/pegtl/internal/pegtl_string.hpp>
#include <tao/pegtl/rules.hpp>

#include <L2.h>
#include <helper.h>
#include <parser.h>
#include <spiller.h>

using namespace TAO_PEGTL_NAMESPACE;

namespace L2 {

class ItemStack {
public:
  ItemStack() : items{std::vector<Item *>()} { return; }

  void push(Item *item) { items.push_back(item); }

  Item *pop() {
    auto item = items.back();
    items.pop_back();
    return item;
  }

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

struct spaces : star<sor<one<' '>, one<'\t'>>> {};

// seps::= (\s*eol)*
struct seps : star<seq<spaces, eol>> {};

// seps_with_comments::= (\s*(eol|comment))*
struct seps_with_comments : star<seq<spaces, sor<eol, comment>>> {};

/*
 * Keywords.
 */
struct ret : TAO_PEGTL_STRING("return") {};
struct arrow : TAO_PEGTL_STRING("<-") {};
// ") <- this is used to fix the colorization

struct r8 : TAO_PEGTL_STRING("r8") {};
struct r9 : TAO_PEGTL_STRING("r9") {};
struct rax : TAO_PEGTL_STRING("rax") {};
struct rcx : TAO_PEGTL_STRING("rcx") {};
struct rdx : TAO_PEGTL_STRING("rdx") {};
struct rdi : TAO_PEGTL_STRING("rdi") {};
struct rsi : TAO_PEGTL_STRING("rsi") {};
struct rsp : TAO_PEGTL_STRING("rsp") {};

struct less_than : TAO_PEGTL_STRING("<") {};
struct less_equal : TAO_PEGTL_STRING("<=") {};
struct equal : TAO_PEGTL_STRING("=") {};

struct lshift : TAO_PEGTL_STRING("<<=") {};
struct rshift : TAO_PEGTL_STRING(">>=") {};

struct self_add : TAO_PEGTL_STRING("+=") {};
struct self_sub : TAO_PEGTL_STRING("-=") {};
struct self_mul : TAO_PEGTL_STRING("*=") {};
struct self_and : TAO_PEGTL_STRING("&=") {};

struct self_inc : TAO_PEGTL_STRING("++") {};
struct self_dec : TAO_PEGTL_STRING("--") {};

struct mem : TAO_PEGTL_STRING("mem") {};

struct call : TAO_PEGTL_STRING("call") {};

struct print : TAO_PEGTL_STRING("print") {};
struct input : TAO_PEGTL_STRING("input") {};
struct allocate : TAO_PEGTL_STRING("allocate") {};
struct tuple_error : TAO_PEGTL_STRING("tuple-error") {};
struct tensor_error : TAO_PEGTL_STRING("tensor-error") {};

struct goto_str : TAO_PEGTL_STRING("goto") {};
struct cjump : TAO_PEGTL_STRING("cjump") {};

struct stack_arg : TAO_PEGTL_STRING("stack-arg") {};

// N ::= (+|-)?[1-9][0-9]* | 0
struct N : seq<opt<sor<one<'-'>, one<'+'>>>, plus<digit>> {};

struct param_num : N {};

// M ::= multiple of 8
struct M : N {};

// label ::= name
struct label : seq<one<':'>, name> {};

// I ::= @name    # function names
struct I : seq<one<'@'>, name> {};

// var ::= %name
struct var : seq<one<'%'>, name> {};

struct spilled_var : var {};
struct spill_prefix : var {};

// sx ::= rcx | var
struct sx : sor<var, rcx> {};

struct callee_func : I {};

// a ::= rdi | rsi | rdx | sx | r8 | r9   # argument registers
struct a : sor<rdi, rsi, rdx, sx, r8, r9> {};

// w ::= a | rax    # general purpose registers
struct w : sor<a, rax> {};

// x ::= w | rsp    # all registers
struct x : sor<w, rsp> {};

// F ::= 1 | 3 | 4    # tensor error parameter number
struct F : sor<one<'1'>, one<'3'>, one<'4'>> {};

// E ::= 1 | 2 | 4 | 8    # scalar size
struct E : sor<one<'1'>, one<'2'>, one<'4'>, one<'8'>> {};

/*
 * Combined rules.
 */
struct cmp_op : sor<less_equal, less_than, equal> {};

// sop ::= <<= | =>>    # shift operators
struct sop : sor<lshift, rshift> {};

// aop ::= += | -= | *= | &=    # arithmetic operators
struct aop : sor<self_add, self_sub, self_mul, self_and> {};

// u ::= w | I    # callee
struct u : sor<callee_func, w> {};

// t ::= x | N    # right value in arithmetic instructions
struct t : sor<x, N> {};

// s ::= t | label | I    # can be used as right value in assignment instructions
struct s : sor<t, label, callee_func> {};

// mem_loc ::= mem x M    # memory location
struct mem_loc : seq<mem, spaces, x, spaces, M> {};

// stack_loc ::= stack-arg M    # stack location
struct stack_loc : seq<stack_arg, spaces, M> {};

struct shift_inst : seq<w, spaces, sop, spaces, sor<sx, N>> {};

struct arith_inst
    : sor<seq<w, spaces, aop, spaces, t>, seq<mem_loc, spaces, sor<self_add, self_sub>, spaces, t>,
          seq<w, spaces, sor<self_add, self_sub>, spaces, mem_loc>> {};

struct self_mod_inst : seq<w, spaces, sor<self_inc, self_dec>> {};

struct norm_assign_inst : sor<seq<w, spaces, arrow, spaces, sor<s, mem_loc, stack_loc>>,
                              seq<mem_loc, spaces, arrow, spaces, s>> {};

struct cmp_assign_inst : seq<w, spaces, arrow, spaces, t, spaces, cmp_op, spaces, t> {};

struct assign_inst : sor<cmp_assign_inst, norm_assign_inst> {};

struct call_inst : seq<call, spaces, u, spaces, N> {};

struct print_inst : seq<call, spaces, print, spaces, one<'1'>> {};

struct input_inst : seq<call, spaces, input, spaces, one<'0'>> {};

struct allocate_inst : seq<call, spaces, allocate, spaces, one<'2'>> {};

struct tuple_error_inst : seq<call, spaces, tuple_error, spaces, one<'3'>> {};

struct tensor_error_inst : seq<call, spaces, tensor_error, spaces, F> {};

struct set_inst : seq<w, spaces, one<'@'>, spaces, w, spaces, w, spaces, E> {};

struct label_inst : label {};

struct goto_inst : seq<goto_str, spaces, label> {};

struct cjump_inst : seq<cjump, spaces, t, spaces, cmp_op, spaces, t, spaces, label> {};

struct instruction
    : sor<seq<at<ret>, ret>, seq<at<assign_inst>, assign_inst>, seq<at<label_inst>, label_inst>,
          seq<at<comment>, comment>, seq<at<shift_inst>, shift_inst>,
          seq<at<arith_inst>, arith_inst>, seq<at<self_mod_inst>, self_mod_inst>,
          seq<at<call_inst>, call_inst>, seq<at<print_inst>, print_inst>,
          seq<at<input_inst>, input_inst>, seq<at<allocate_inst>, allocate_inst>,
          seq<at<tuple_error_inst>, tuple_error_inst>,
          seq<at<tensor_error_inst>, tensor_error_inst>, seq<at<set_inst>, set_inst>,
          seq<at<goto_inst>, goto_inst>, seq<at<cjump_inst>, cjump_inst>> {};

struct instructions : plus<seq<seps, bol, spaces, instruction, seps>> {};

struct function : seq<seq<spaces, one<'('>>, seps_with_comments, seq<spaces, I>, seps_with_comments,
                      seq<spaces, param_num>, seps_with_comments, instructions, seps_with_comments,
                      seq<spaces, one<')'>>> {};

struct functions : plus<seps_with_comments, function, seps_with_comments> {};

struct func_only : seq<seps_with_comments, function, seps_with_comments> {};

struct spill_func : seq<seps_with_comments, function, seps_with_comments, spilled_var,
                        seps_with_comments, spill_prefix, seps_with_comments> {};

struct entry_point
    : seq<seps_with_comments, seq<spaces, one<'('>>, seps_with_comments, I, seps_with_comments,
          functions, seps_with_comments, seq<spaces, one<')'>>, seps> {};

struct grammar : must<entry_point> {};

struct func_grammar : must<func_only> {};

struct spill_grammar : must<spill_func> {};

/*
 * Actions attached to grammar rules.
 */
template <typename Rule> struct action : nothing<Rule> {};

template <> struct action<callee_func> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto n = new FunctionName(in.string());
    itemStack.push(n);
  }
};

template <> struct action<I> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto name = in.string();
    if (P.getEntryPointLabel().empty()) {
      P.setEntryPointLabel(name);
    } else {
      auto newF = new Function(name);
      P.addFunction(newF);
    }
  }
};

template <> struct action<var> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto F = P.getCurrFunction();
    auto var = F->getVariable(in.string());
    itemStack.push(var);
  }
};

template <> struct action<spilled_var> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto &prog = *((ProgramToSpill *)(&P));
    prog.setSpilledVar(in.string());
  }
};

template <> struct action<spill_prefix> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto &prog = *((ProgramToSpill *)(&P));
    prog.setSpillPrefix(in.string());
  }
};

template <> struct action<N> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto n = new Number(std::stoll(in.string()));
    itemStack.push(n);
  }
};

template <> struct action<param_num> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto currF = P.getCurrFunction();
    currF->setParameters(std::stoll(in.string()));
  }
};

// offset number: multiple of 8
template <> struct action<M> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto offset = std::stoll(in.string());
    if (offset % 8 != 0)
      throw parse_error("Offset number must be a multiple of 8.", in);

    auto n = new Number(offset);
    itemStack.push(n);
  }
};

template <> struct action<F> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto n = new Number(std::stoll(in.string()));
    itemStack.push(n);
  }
};

template <> struct action<E> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto n = new Number(std::stoll(in.string()));
    itemStack.push(n);
  }
};

template <> struct action<ret> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto I = new RetInst();
    auto currBB = P.getCurrFunction()->getCurrBasicBlock();
    currBB->addInstruction(I);

    // next instruction is in a new BB, but is not a successor of currBB
    auto newBB = new BasicBlock();
    P.getCurrFunction()->addBasicBlock(newBB);
  }
};

template <> struct action<label> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto l = new Label(in.string());
    itemStack.push(l);
  }
};

template <> struct action<r8> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto r = Register::getRegister(Register::ID::R8);
    itemStack.push(r);
  }
};

template <> struct action<r9> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto r = Register::getRegister(Register::ID::R9);
    itemStack.push(r);
  }
};

template <> struct action<rax> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto r = Register::getRegister(Register::ID::RAX);
    itemStack.push(r);
  }
};

template <> struct action<rcx> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto r = Register::getRegister(Register::ID::RCX);
    itemStack.push(r);
  }
};

template <> struct action<rdx> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto r = Register::getRegister(Register::ID::RDX);
    itemStack.push(r);
  }
};

template <> struct action<rdi> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto r = Register::getRegister(Register::ID::RDI);
    itemStack.push(r);
  }
};

template <> struct action<rsi> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto r = Register::getRegister(Register::ID::RSI);
    itemStack.push(r);
  }
};

template <> struct action<rsp> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto r = Register::getRegister(Register::ID::RSP);
    itemStack.push(r);
  }
};

template <> struct action<less_than> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto op = CompareOp::getCompareOp(CompareOp::ID::LESS_THAN);
    itemStack.push(op);
  }
};

template <> struct action<less_equal> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto op = CompareOp::getCompareOp(CompareOp::ID::LESS_EQUAL);
    itemStack.push(op);
  }
};

template <> struct action<equal> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto op = CompareOp::getCompareOp(CompareOp::ID::EQUAL);
    itemStack.push(op);
  }
};

template <> struct action<lshift> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto op = ShiftOp::getShiftOp(ShiftOp::ID::LEFT);
    itemStack.push(op);
  }
};

template <> struct action<rshift> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto op = ShiftOp::getShiftOp(ShiftOp::ID::RIGHT);
    itemStack.push(op);
  }
};

template <> struct action<self_add> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto op = ArithOp::getArithOp(ArithOp::ID::ADD);
    itemStack.push(op);
  }
};

template <> struct action<self_sub> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto op = ArithOp::getArithOp(ArithOp::ID::SUB);
    itemStack.push(op);
  }
};

template <> struct action<self_mul> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto op = ArithOp::getArithOp(ArithOp::ID::MUL);
    itemStack.push(op);
  }
};

template <> struct action<self_and> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto op = ArithOp::getArithOp(ArithOp::ID::AND);
    itemStack.push(op);
  }
};

template <> struct action<self_inc> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto op = SelfModOp::getSelfModOp(SelfModOp::ID::INC);
    itemStack.push(op);
  }
};

template <> struct action<self_dec> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto op = SelfModOp::getSelfModOp(SelfModOp::ID::DEC);
    itemStack.push(op);
  }
};

template <> struct action<mem_loc> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    auto offset = (Number *)itemStack.pop();
    auto base = (Symbol *)itemStack.pop();
    auto m = new MemoryLocation(base, offset);
    itemStack.push(m);
  }
};

template <> struct action<stack_loc> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    debug("parsing stack_loc");
    auto offset = (Number *)itemStack.pop();
    auto s = new StackLocation(offset);
    itemStack.push(s);
  }
};

template <> struct action<shift_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    debug("parsing shift_inst");
    auto rval = (Value *)itemStack.pop();
    auto op = (ShiftOp *)itemStack.pop();
    auto lval = (Symbol *)itemStack.pop();
    auto I = new ShiftInst(op, lval, rval);
    auto currBB = P.getCurrFunction()->getCurrBasicBlock();
    currBB->addInstruction(I);
  }
};

template <> struct action<arith_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    debug("parsing arith_inst");
    auto rval = itemStack.pop();
    auto op = (ArithOp *)itemStack.pop();
    auto lval = itemStack.pop();
    auto I = new ArithInst(op, lval, rval);
    auto currBB = P.getCurrFunction()->getCurrBasicBlock();
    currBB->addInstruction(I);
  }
};

template <> struct action<self_mod_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    debug("parsing self_mod_inst");
    auto op = (SelfModOp *)itemStack.pop();
    auto lval = (Symbol *)itemStack.pop();
    auto I = new SelfModInst(op, lval);
    auto currBB = P.getCurrFunction()->getCurrBasicBlock();
    currBB->addInstruction(I);
  }
};

template <> struct action<norm_assign_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    debug("parsing norm_assign_inst");
    auto rval = itemStack.pop();
    auto lval = itemStack.pop();
    auto I = new AssignInst(lval, rval);
    auto currBB = P.getCurrFunction()->getCurrBasicBlock();
    currBB->addInstruction(I);
  }
};

template <> struct action<cmp_assign_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    debug("parsing cmp_assign_inst");
    auto cmpRval = (Value *)itemStack.pop();
    auto op = (CompareOp *)itemStack.pop();
    auto cmpLval = (Value *)itemStack.pop();
    auto lval = (Symbol *)itemStack.pop();
    auto I = new CompareAssignInst(lval, op, cmpLval, cmpRval);
    auto currBB = P.getCurrFunction()->getCurrBasicBlock();
    currBB->addInstruction(I);
  }
};

template <> struct action<call_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    debug("parsing call_inst");
    auto argNum = (Number *)itemStack.pop();
    auto callee = itemStack.pop();
    auto I = new CallInst(callee, argNum);
    auto currBB = P.getCurrFunction()->getCurrBasicBlock();
    currBB->addInstruction(I);
  }
};

template <> struct action<print_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    debug("parsing print_inst");
    auto I = new PrintInst();
    auto currBB = P.getCurrFunction()->getCurrBasicBlock();
    currBB->addInstruction(I);
  }
};

template <> struct action<input_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    debug("parsing input_inst");
    auto I = new InputInst();
    auto currBB = P.getCurrFunction()->getCurrBasicBlock();
    currBB->addInstruction(I);
  }
};

template <> struct action<allocate_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    debug("parsing allocate_inst");
    auto I = new AllocateInst();
    auto currBB = P.getCurrFunction()->getCurrBasicBlock();
    currBB->addInstruction(I);
  }
};

template <> struct action<tuple_error_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    debug("parsing tuple_error_inst");
    auto I = new TupleErrorInst();
    auto currBB = P.getCurrFunction()->getCurrBasicBlock();
    currBB->addInstruction(I);

    // act like return, new BB
    auto newBB = new BasicBlock();
    P.getCurrFunction()->addBasicBlock(newBB);
  }
};

template <> struct action<tensor_error_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    debug("parsing tensor_error_inst");
    auto number = (Number *)itemStack.pop();
    auto I = new TensorErrorInst(number);
    auto currBB = P.getCurrFunction()->getCurrBasicBlock();
    currBB->addInstruction(I);

    // act like return, new BB
    auto newBB = new BasicBlock();
    P.getCurrFunction()->addBasicBlock(newBB);
  }
};

template <> struct action<set_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    debug("parsing set_inst");
    auto scalar = (Number *)itemStack.pop();
    auto offset = (Symbol *)itemStack.pop();
    auto base = (Symbol *)itemStack.pop();
    auto lval = (Symbol *)itemStack.pop();
    auto I = new SetInst(lval, base, offset, scalar);
    auto currBB = P.getCurrFunction()->getCurrBasicBlock();
    currBB->addInstruction(I);
  }
};

template <> struct action<label_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    debug("parsing label_inst");
    auto label = new Label(in.string());
    auto I = new LabelInst(label);

    auto currBB = P.getCurrFunction()->getCurrBasicBlock();

    if (!currBB->getInstructions().empty()) {
      // last instruction is not goto or cjump
      auto newBB = new BasicBlock();
      newBB->addPredecessor(currBB);
      currBB->addSuccessor(newBB);
      P.getCurrFunction()->addBasicBlock(newBB);
      currBB = newBB;
    }

    currBB->addInstruction(I);
  }
};

template <> struct action<goto_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    debug("parsing goto_inst");
    auto label = (Label *)itemStack.pop();
    auto I = new GotoInst(label);
    auto currBB = P.getCurrFunction()->getCurrBasicBlock();
    currBB->addInstruction(I);

    // next instruction is in a new basic block
    auto newBB = new BasicBlock();
    P.getCurrFunction()->addBasicBlock(newBB);
  }
};

template <> struct action<cjump_inst> {
  template <typename Input> static void apply(const Input &in, Program &P) {
    debug("parsing cjump_inst");
    auto label = (Label *)itemStack.pop();
    auto rval = (Value *)itemStack.pop();
    auto op = (CompareOp *)itemStack.pop();
    auto lval = (Value *)itemStack.pop();
    auto I = new CondJumpInst(op, lval, rval, label);
    auto currBB = P.getCurrFunction()->getCurrBasicBlock();
    currBB->addInstruction(I);

    // next instruction is in a new BB, and is a successor of currBB
    auto newBB = new BasicBlock();
    newBB->addPredecessor(currBB);
    currBB->addSuccessor(newBB);
    P.getCurrFunction()->addBasicBlock(newBB);
  }
};

void linkBasicBlocks(Function *F) {
  debug("Started linking basic blocks.");

  // emit empty basic blocks that may at the end of the func
  auto lastBB = F->getCurrBasicBlock();
  if (lastBB->getInstructions().empty()) {
    for (auto pred : lastBB->getPredecessors())
      pred->removeSuccessor(lastBB);
    F->popCurrBasicBlock();
  }

  std::map<std::string, BasicBlock *> labelToBB;
  // find all basic blocks that starts with a label
  // these BBs may have predecessors that are not linked yet
  for (auto &BB : F->getBasicBlocks())
    if (auto inst = dynamic_cast<LabelInst *>(BB->getFirstInstruction()))
      labelToBB[inst->getLabel()->getName()] = BB;

  // link all basic blocks
  for (auto &BB : F->getBasicBlocks()) {
    if (auto inst = dynamic_cast<GotoInst *>(BB->getTerminator())) {
      auto label = inst->getLabel();
      auto targetBB = labelToBB[label->getName()];
      BB->addSuccessor(targetBB);
      targetBB->addPredecessor(BB);
    } else if (auto inst = dynamic_cast<CondJumpInst *>(BB->getTerminator())) {
      auto label = inst->getLabel();
      auto targetBB = labelToBB[label->getName()];
      BB->addSuccessor(targetBB);
      targetBB->addPredecessor(BB);
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
  for (auto F : P->getFunctions())
    linkBasicBlocks(F);
  return P;
}

ProgramToSpill *parseSpillFile(char *fileName) {
  if (analyze<spill_grammar>() != 0) {
    std::cerr << "There are problems with the grammar" << std::endl;
    exit(1);
  }

  file_input<> fileInput(fileName);
  auto P = new ProgramToSpill();
  parse<spill_grammar, action>(fileInput, *P);
  return P;
}

Program *parseFunctionFile(char *fileName) {
  /*
   * Check the grammar for some possible issues.
   */
  if (analyze<func_grammar>() != 0) {
    std::cerr << "There are problems with the grammar" << std::endl;
    exit(1);
  }

  /*
   * Parse.
   */
  file_input<> fileInput(fileName);
  auto P = new Program();
  P->setEntryPointLabel("@<Phony>");
  parse<func_grammar, action>(fileInput, *P);
  for (auto F : P->getFunctions())
    linkBasicBlocks(F);
  return P;
}

} // namespace L2
