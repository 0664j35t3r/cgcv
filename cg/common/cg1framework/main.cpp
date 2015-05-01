#include <iostream>
#include <memory>

#include "cfg_testcase.h"
#include "config_utils.h"

using namespace CgcvConfig;

extern const char* taskname;
extern const char* version_string;

void run_testcase(CgcvConfig::Testcase* testcase, const std::string& output, int width, int height);

int main(int argc, char* argv[])
{
  if (argc != 2)
  {
    std::cout << "Usage: ./" << taskname << " <config-file>\n" << std::endl;
    return -1;
  }

  std::cout << version_string << std::endl; // DO NOT REMOVE THIS LINE!!!

  try
  {
    std::auto_ptr<Config> cfg(Config::open(argv[1]));
    size_t numTestcases = cfg->getNumElements("testcase");
    for (size_t i = 0; i < numTestcases; ++i)
    {
      Testcase* testcase = cfg->getTestcase(i);
      std::cout << "running testcase " << testcase->getAttribute<std::string>("name") << std::endl;

      std::string output = getAttribute<std::string>(*testcase, "output", "output.png");
      int width = getAttribute<int>(*testcase, "width", 800);
      int height = getAttribute<int>(*testcase, "height", 600);

      run_testcase(testcase, output, width, height);
    }
  }
  catch (std::exception& e)
  {
    std::cout << "error: " << e.what() << std::endl;
    return -1;
  }

  return 0;
}
