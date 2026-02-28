#ifndef LIB_ARGPARSE_
#define LIB_ARGPARSE_

#include <map>
#include <string>
#include <optional>
#include <tuple>
#include <variant>

enum class Arg {
  Optional,
  Required,
};

using ArgParseVariant = std::variant<int, bool, std::string>;
using ArgParseTuple = std::tuple<Arg, std::string, std::optional<ArgParseVariant>, bool>;

class ArgParse final {
 public:

  ArgParse (const ArgParse&) = delete;
  ArgParse& operator=(const ArgParse&) = delete;

  static ArgParse& getInstance()
  {
    static ArgParse instance;
    return instance;
  }

  template<typename T>
  void add(const Arg arg, const char name, const char* desc);

  template<typename T>
  static std::optional<T> getArg(const char name);

  static void parse(int argc, char **argv);

 private:

  ArgParse() {};

  static void printHelp(char **argv);

  static std::map<char, ArgParseTuple> args;

};

template<typename T>
void ArgParse::add(const Arg arg, const char name, const char* desc)
{
  // Temporarly assign default constructed value
  // so to be able to determin the type during parse.
  const bool userSet = false;
  std::optional<ArgParseVariant> opt = T{};
  auto & val = opt.value();
  if (std::holds_alternative<bool>(val))
    val = false;

  args[name] = std::make_tuple(arg, desc, opt, userSet);
}

template<typename T>
std::optional<T> ArgParse::getArg(const char name)
{
  if (auto search = args.find(name); search != args.end()) {
    const auto & [key, desc, opt, userSet] = search->second;
    if (userSet)
      return std::get<T>(opt.value());
  }

  return std::nullopt;
}

#endif //LIB_ARGPARSE_
