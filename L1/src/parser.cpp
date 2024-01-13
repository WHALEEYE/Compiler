/*
 * SUGGESTION FROM THE CC TEAM:
 * double check the order of actions that are fired.
 * You can do this in (at least) two ways:
 * 1) by using gdb adding breakpoints to actions
 * 2) by adding printing statements in each action
 *
 * For 2), we suggest writing the code to make it straightforward to
 * enable/disable all of them (e.g., assuming shouldIPrint is a global variable
 *    if (shouldIPrint) std::cerr << "MY OUTPUT" << std::endl;
 * )
 */
#include <algorithm>
#include <assert.h>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <sched.h>
#include <set>
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>

#include <tao/pegtl.hpp>
#include <tao/pegtl/contrib/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>

#include <L1.h>
#include <parser.h>

using namespace TAO_PEGTL_NAMESPACE;

namespace L1 {

/*
 * Tokens parsed
 */
std::vector<Item *> parsed_items;

/*
 * Grammar rules from now on.
 */
struct name
    : seq<plus<sor<alpha, one<'_'>>>, star<sor<alpha, one<'_'>, digit>>> {};

/*
 * Keywords.
 */
struct str_return : TAO_PEGTL_STRING("return") {};
struct str_arrow : TAO_PEGTL_STRING("<-") {};

// " <- this is used to fix the colorization

struct str_rdi : TAO_PEGTL_STRING("rdi") {};
struct str_rax : TAO_PEGTL_STRING("rax") {};

struct label : seq<one<':'>, name> {};

struct function_name_rule : seq<one<'@'>, name> {};

struct register_rdi_rule : str_rdi {};

struct register_rax_rule : str_rax {};

struct register_rule : sor<register_rdi_rule, register_rax_rule> {};

struct number : seq<opt<sor<one<'-'>, one<'+'>>>, plus<digit>> {};

struct function_name : label {};

struct argument_number : number {};

struct local_number : number {};

struct comment : disable<TAO_PEGTL_STRING("//"), until<eolf>> {};

/*
 * Separators.
 */
struct spaces : star<sor<one<' '>, one<'\t'>>> {};

struct seps : star<seq<spaces, eol>> {};
struct seps_with_comments : star<seq<spaces, sor<eol, comment>>> {};

struct Instruction_return_rule : seq<str_return> {};

struct Instruction_assignment_rule
    : seq<register_rule, spaces, str_arrow, spaces, register_rule> {};

struct Instruction_rule
    : sor<seq<at<Instruction_return_rule>, Instruction_return_rule>,
          seq<at<Instruction_assignment_rule>, Instruction_assignment_rule>,
          seq<at<comment>, comment>> {};

struct Instructions_rule
    : plus<seq<seps, bol, spaces, Instruction_rule, seps>> {};

struct Function_rule
    : seq<seq<spaces, one<'('>>, seps_with_comments,
          seq<spaces, function_name_rule>, seps_with_comments,
          seq<spaces, argument_number>, seps_with_comments,
          seq<spaces, local_number>, seps_with_comments, Instructions_rule,
          seps_with_comments, seq<spaces, one<')'>>> {};

struct Functions_rule
    : plus<seps_with_comments, Function_rule, seps_with_comments> {};

struct entry_point_rule
    : seq<seps_with_comments, seq<spaces, one<'('>>, seps_with_comments,
          function_name_rule, seps_with_comments, Functions_rule,
          seps_with_comments, seq<spaces, one<')'>>, seps> {};

struct grammar : must<entry_point_rule> {};

/*
 * Actions attached to grammar rules.
 */
template <typename Rule> struct action : nothing<Rule> {};

template <> struct action<function_name_rule> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    if (p.entryPointLabel.empty()) {
      p.entryPointLabel = in.string();
    } else {
      auto newF = new Function();
      newF->name = in.string();
      p.functions.push_back(newF);
    }
  }
};

template <> struct action<argument_number> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto currentF = p.functions.back();
    currentF->arguments = std::stoll(in.string());
  }
};

template <> struct action<local_number> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto currentF = p.functions.back();
    currentF->locals = std::stoll(in.string());
  }
};

template <> struct action<str_return> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto currentF = p.functions.back();
    auto i = new Instruction_ret();
    currentF->instructions.push_back(i);
  }
};

template <> struct action<register_rdi_rule> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto r = new Register(RegisterID::rdi);
    parsed_items.push_back(r);
  }
};

template <> struct action<register_rax_rule> {
  template <typename Input> static void apply(const Input &in, Program &p) {
    auto r = new Register(RegisterID::rax);
    parsed_items.push_back(r);
  }
};

template <> struct action<Instruction_assignment_rule> {
  template <typename Input> static void apply(const Input &in, Program &p) {

    /*
     * Fetch the current function.
     */
    auto currentF = p.functions.back();

    /*
     * Fetch the last two tokens parsed.
     */
    auto src = parsed_items.back();
    parsed_items.pop_back();
    auto dst = parsed_items.back();
    parsed_items.pop_back();

    /*
     * Create the instruction.
     */
    auto i = new Instruction_assignment(dst, src);

    /*
     * Add the just-created instruction to the current function.
     */
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

} // namespace L1
