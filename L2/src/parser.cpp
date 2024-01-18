#include "tao/pegtl/ascii.hpp"
#include "tao/pegtl/internal/pegtl_string.hpp"
#include "tao/pegtl/rules.hpp"
#include <iostream>
#include <vector>

#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>

#include <L2.h>
#include <parser.h>

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

struct small : TAO_PEGTL_STRING("<") {};
struct sm_eq : TAO_PEGTL_STRING("<=") {};
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

struct number : seq<opt<sor<one<'-'>, one<'+'>>>, plus<digit>> {};

struct param_num : number {};

struct offset_num : number {};

struct label : seq<one<':'>, name> {};

struct function_name : seq<one<'@'>, name> {};

struct var_name : seq<one<'%'>, name> {};

struct ref_func_name : function_name {};

struct shift_rval : sor<rcx, var_name, number> {};

struct arg_regs : sor<rdi, rsi, rdx, shift_rval, r8, r9> {};

struct usr_vars : sor<arg_regs, rax> {};

struct all_vars : sor<usr_vars, rsp> {};

struct tensor_error_num : sor<one<'1'>, one<'3'>, one<'4'>> {};

struct scalar_num : sor<one<'1'>, one<'2'>, one<'4'>, one<'8'>> {};

/*
 * Combined rules.
 */
struct cmp_op : sor<sm_eq, small, equal> {};

struct shift_op : sor<lshift, rshift> {};

struct arith_op : sor<self_add, self_sub, self_mul, self_and> {};

struct callee : sor<ref_func_name, usr_vars> {};

struct arith_rval : sor<all_vars, number> {};

struct direct_val : sor<arith_rval, label, ref_func_name> {};

struct mem_loc : seq<mem, spaces, all_vars, spaces, offset_num> {};

struct stack_loc : seq<stack_arg, spaces, offset_num> {};

struct shift_inst : seq<usr_vars, spaces, shift_op, spaces, shift_rval> {};

struct arith_inst : sor<seq<usr_vars, spaces, arith_op, spaces, arith_rval>,
                        seq<mem_loc, spaces, sor<self_add, self_sub>, spaces, arith_rval>,
                        seq<usr_vars, spaces, arith_op, spaces, mem_loc>> {};

struct self_mod_inst : seq<usr_vars, spaces, sor<self_inc, self_dec>> {};

struct norm_assign_inst
    : sor<seq<usr_vars, spaces, arrow, spaces, sor<direct_val, mem_loc, stack_loc>>,
          seq<mem_loc, spaces, arrow, spaces, direct_val>> {};

struct cmp_assign_inst
    : seq<usr_vars, spaces, arrow, spaces, arith_rval, spaces, cmp_op, spaces, arith_rval> {};

struct assign_inst : sor<cmp_assign_inst, norm_assign_inst> {};

struct call_inst : seq<call, spaces, callee, spaces, number> {};

struct print_inst : seq<call, spaces, print, spaces, one<'1'>> {};

struct input_inst : seq<call, spaces, input, spaces, one<'0'>> {};

struct allocate_inst : seq<call, spaces, allocate, spaces, one<'2'>> {};

struct tuple_error_inst : seq<call, spaces, tuple_error, spaces, one<'3'>> {};

struct tensor_error_inst : seq<call, spaces, tensor_error, spaces, tensor_error_num> {};

struct set_inst
    : seq<usr_vars, spaces, one<'@'>, spaces, usr_vars, spaces, usr_vars, spaces, scalar_num> {};

struct label_inst : label {};

struct goto_inst : seq<goto_str, spaces, label> {};

struct cjump_inst
    : seq<cjump, spaces, arith_rval, spaces, cmp_op, spaces, arith_rval, spaces, label> {};

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

struct function : seq<seq<spaces, one<'('>>, seps_with_comments, seq<spaces, function_name>,
                      seps_with_comments, seq<spaces, param_num>, seps_with_comments, instructions,
                      seps_with_comments, seq<spaces, one<')'>>> {};

struct functions : plus<seps_with_comments, function, seps_with_comments> {};

struct entry_point
    : seq<seps_with_comments, seq<spaces, one<'('>>, seps_with_comments, function_name,
          seps_with_comments, functions, seps_with_comments, seq<spaces, one<')'>>, seps> {};

struct grammar : must<entry_point> {};

/*
 * Actions attached to grammar rules.
 */
template <typename Rule> struct action : nothing<Rule> {};

template <> struct action<ref_func_name> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto n = new FunctionName(in.string());
    itemStack.push(n);
  }
};

template <> struct action<function_name> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto name = in.string();
    if (p.entryPointLabel.empty()) {
      p.entryPointLabel = name;
    } else {
      auto newF = new Function();
      newF->name = name;
      p.functions.push_back(newF);
    }
  }
};

template <> struct action<var_name> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto n = new Variable(in.string());
    itemStack.push(n);
  }
};

template <> struct action<number> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto n = new Number(std::stoll(in.string()));
    itemStack.push(n);
  }
};

template <> struct action<param_num> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto currentF = p.functions.back();
    currentF->parameters = std::stoll(in.string());
  }
};

// offset number: multiple of 8
template <> struct action<offset_num> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto offset = std::stoll(in.string());
    if (offset % 8 != 0)
      throw parse_error("Offset number must be a multiple of 8.", in);

    auto n = new Number(offset);
    itemStack.push(n);
  }
};

template <> struct action<tensor_error_num> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto n = new Number(std::stoll(in.string()));
    itemStack.push(n);
  }
};

template <> struct action<scalar_num> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto n = new Number(std::stoll(in.string()));
    itemStack.push(n);
  }
};

template <> struct action<ret> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto currentF = p.functions.back();
    auto i = new RetInst();
    currentF->instructions.push_back(i);
  }
};

template <> struct action<label> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto l = new Label(in.string());
    itemStack.push(l);
  }
};

template <> struct action<r8> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto r = new Register(RegisterID::R8);
    itemStack.push(r);
  }
};

template <> struct action<r9> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto r = new Register(RegisterID::R9);
    itemStack.push(r);
  }
};

template <> struct action<rax> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto r = new Register(RegisterID::RAX);
    itemStack.push(r);
  }
};

template <> struct action<rcx> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto r = new Register(RegisterID::RCX);
    itemStack.push(r);
  }
};

template <> struct action<rdx> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto r = new Register(RegisterID::RDX);
    itemStack.push(r);
  }
};

template <> struct action<rdi> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto r = new Register(RegisterID::RDI);
    itemStack.push(r);
  }
};

template <> struct action<rsi> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto r = new Register(RegisterID::RSI);
    itemStack.push(r);
  }
};

template <> struct action<rsp> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto r = new Register(RegisterID::RSP);
    itemStack.push(r);
  }
};

template <> struct action<small> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto cmp = new CompareOp(CompareOpID::LESS_THAN);
    itemStack.push(cmp);
  }
};

template <> struct action<sm_eq> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto cmp = new CompareOp(CompareOpID::LESS_EQUAL);
    itemStack.push(cmp);
  }
};

template <> struct action<equal> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto cmp = new CompareOp(CompareOpID::EQUAL);
    itemStack.push(cmp);
  }
};

template <> struct action<lshift> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto op = new ShiftOp(ShiftOpID::LEFT);
    itemStack.push(op);
  }
};

template <> struct action<rshift> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto op = new ShiftOp(ShiftOpID::RIGHT);
    itemStack.push(op);
  }
};

template <> struct action<self_add> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto op = new ArithOp(ArithOpID::ADD);
    itemStack.push(op);
  }
};

template <> struct action<self_sub> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto op = new ArithOp(ArithOpID::SUB);
    itemStack.push(op);
  }
};

template <> struct action<self_mul> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto op = new ArithOp(ArithOpID::MUL);
    itemStack.push(op);
  }
};

template <> struct action<self_and> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto op = new ArithOp(ArithOpID::AND);
    itemStack.push(op);
  }
};

template <> struct action<self_inc> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto op = new SelfModOp(SelfModOpID::INC);
    itemStack.push(op);
  }
};

template <> struct action<self_dec> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto op = new SelfModOp(SelfModOpID::DEC);
    itemStack.push(op);
  }
};

template <> struct action<mem_loc> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto offset = (Number *)itemStack.pop();
    auto reg = (Register *)itemStack.pop();
    auto m = new MemoryLocation(reg, offset);
    itemStack.push(m);
  }
};

template <> struct action<stack_loc> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto offset = (Number *)itemStack.pop();
    auto s = new StackLocation(offset);
    itemStack.push(s);
  }
};

template <> struct action<shift_inst> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto rval = itemStack.pop();
    auto op = (ShiftOp *)itemStack.pop();
    auto lval = itemStack.pop();
    auto i = new ShiftInst(op, lval, rval);
    auto currentF = p.functions.back();
    currentF->instructions.push_back(i);
  }
};

template <> struct action<arith_inst> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto rval = itemStack.pop();
    auto op = (ArithOp *)itemStack.pop();
    auto lval = itemStack.pop();
    auto i = new ArithInst(op, lval, rval);
    auto currentF = p.functions.back();
    currentF->instructions.push_back(i);
  }
};

template <> struct action<self_mod_inst> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto op = (SelfModOp *)itemStack.pop();
    auto lval = itemStack.pop();
    auto i = new SelfModInst(op, lval);
    auto currentF = p.functions.back();
    currentF->instructions.push_back(i);
  }
};

template <> struct action<norm_assign_inst> {
  template <typename Input> static void apply(const Input &in, Program &p) {

    /*
     * Fetch the current function.
     */
    auto currentF = p.functions.back();

    /*
     * Fetch the last two tokens parsed.
     */
    auto rval = itemStack.pop();
    auto lval = itemStack.pop();

    /*
     * Create the instruction.
     */
    auto i = new AssignInst(lval, rval);

    /*
     * Add the just-created instruction to the current function.
     */
    currentF->instructions.push_back(i);
  }
};

template <> struct action<cmp_assign_inst> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto cmpRval = itemStack.pop();
    auto op = (CompareOp *)itemStack.pop();
    auto cmpLval = itemStack.pop();
    auto lval = (Register *)itemStack.pop();
    auto i = new CompareAssignInst(lval, op, cmpLval, cmpRval);
    auto currentF = p.functions.back();
    currentF->instructions.push_back(i);
  }
};

template <> struct action<call_inst> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto arg_num = (Number *)itemStack.pop();
    auto callee = itemStack.pop();
    auto i = new CallInst(callee, arg_num);
    auto currentF = p.functions.back();
    currentF->instructions.push_back(i);
  }
};

template <> struct action<print_inst> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto i = new PrintInst();
    auto currentF = p.functions.back();
    currentF->instructions.push_back(i);
  }
};

template <> struct action<input_inst> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto i = new InputInst();
    auto currentF = p.functions.back();
    currentF->instructions.push_back(i);
  }
};

template <> struct action<allocate_inst> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto i = new AllocateInst();
    auto currentF = p.functions.back();
    currentF->instructions.push_back(i);
  }
};

template <> struct action<tuple_error_inst> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto i = new TupleErrorInst();
    auto currentF = p.functions.back();
    currentF->instructions.push_back(i);
  }
};

template <> struct action<tensor_error_inst> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto number = (Number *)itemStack.pop();
    auto i = new TensorErrorInst(number);
    auto currentF = p.functions.back();
    currentF->instructions.push_back(i);
  }
};

template <> struct action<set_inst> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto scalar = (Number *)itemStack.pop();
    auto offset = (Register *)itemStack.pop();
    auto base = (Register *)itemStack.pop();
    auto lval = (Register *)itemStack.pop();
    auto i = new SetInst(lval, base, offset, scalar);
    auto currentF = p.functions.back();
    currentF->instructions.push_back(i);
  }
};

template <> struct action<label_inst> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto label = new Label(in.string());
    auto i = new LabelInst(label);
    auto currentF = p.functions.back();
    currentF->instructions.push_back(i);
  }
};

template <> struct action<goto_inst> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto label = (Label *)itemStack.pop();
    auto i = new GotoInst(label);
    auto currentF = p.functions.back();
    currentF->instructions.push_back(i);
  }
};

template <> struct action<cjump_inst> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto label = (Label *)itemStack.pop();
    auto rval = itemStack.pop();
    auto op = (CompareOp *)itemStack.pop();
    auto lval = itemStack.pop();
    auto i = new CondJumpInst(op, lval, rval, label);
    auto currentF = p.functions.back();
    currentF->instructions.push_back(i);
  }
};

Program parse_file(char *fileName) {

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
  Program p;
  parse<grammar, action>(fileInput, p);

  return p;
}

Program parse_spill_file(char *fileName) {
  throw std::runtime_error("parse_spill_file() not implemented yet.");
}

Program parse_function_file(char *fileName) {
  /*
   * Check the grammar for some possible issues.
   */
  if (analyze<function>() != 0) {
    std::cerr << "There are problems with the grammar" << std::endl;
    exit(1);
  }

  /*
   * Parse.
   */
  file_input<> fileInput(fileName);
  Program p;
  p.entryPointLabel = "<none>";
  parse<function, action>(fileInput, p);

  return p;
}

} // namespace L2
