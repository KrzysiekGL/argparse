#include <algorithm>
#include <iostream>
#include <list>
#include <sstream>

#include <unistd.h>

#include "ArgParse.hpp"

std::map<char, ArgParseTuple> ArgParse::args;

void ArgParse::printHelp(char **argv)
{
  std::cout << "Usage: " << argv[0] << " {options}\n";
  std::cout << "[-h]\t\tDisplay this help message\n";
  for (const auto & [key, tuple] : args) {
    const auto & [arg, desc, opt, userSet] = tuple;
    std::stringstream ss;

    if (arg == Arg::Required)
      ss << "-" << key;
    else
      ss << "[-" << key << "]";

    if (std::holds_alternative<bool>(opt.value()))
      ss << "\t\t";
    else
      ss << " <value>\t";

    ss << desc;
    std::cout << ss.str() << '\n';
  }
}

void ArgParse::parse(int argc, char ** argv)
{
  // Generate options string expected by getopt(3)
  std::stringstream ss;
  ss << 'h';
  for (const auto & [key, tuple] : args) {
    const auto & [arg, desc, opt, userSet] = tuple;
    if (key != 'h')
      ss << key;
    if (!std::holds_alternative<bool>(opt.value())) // not a flag, so needs a value
      ss << ':';
  }

  std::string argstring;
  ss >> argstring;

  // Iterate over user provided arguments and macth them
  // with those declared. Mark user-set parameters.
  for (size_t i = 0; i < argc; i++) {
    auto key = getopt(argc, argv, argstring.c_str());

    switch (key) {
    case 'h':
    case '?':
      printHelp(argv);
      exit(EXIT_SUCCESS);
    case -1:
      break;
    }

    if (auto search = args.find(key); search != args.end()) {
      auto & [arg, desc, opt, userSet] = search->second;
      auto & val = opt.value();
      if (std::holds_alternative<bool>(val)) {
	val = true;
      } else if (std::holds_alternative<int>(val)) {
	val = atoi(optarg);
      } else if (std::holds_alternative<std::string>(val)) {
	val = std::string(optarg);
      }
      userSet = true;
    }
  }

  // Assert that the required parameters were provided by user.
  // Set val of not required and not set by user variables to nullopt.
  for (auto & [key, tuple] : args) {
    auto & [arg, desc, opt, userSet] = tuple;
    if (arg == Arg::Required && !userSet) {
      printHelp(argv);
      exit(EXIT_FAILURE);
    } else if (!userSet) {
      opt.reset();
    }
  }
}
