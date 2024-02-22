#include <Vtop.h>
#include <iostream>
#include <memory>
#include <verilated.h>

int main(int argc, char **argv)
{
  std::cout << "hello from " << __FILE__ << std::endl;
  const std::unique_ptr<VerilatedContext> contextp{new VerilatedContext};
  contextp->commandArgs(argc, argv);
  const std::unique_ptr<Vtop> top{new Vtop{contextp.get(), "TOP"}};
  while (!contextp->gotFinish())
  {
    contextp->timeInc(1);
    top->eval();
  }
  top->final();
  return 0;
}
