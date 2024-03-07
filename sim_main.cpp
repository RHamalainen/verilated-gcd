#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include <verilated.h>
#if VM_TRACE
#include <verilated_vcd_c.h>
#endif

#include "Vtop.h"

// Simulation time
vluint64_t main_time = 0;
double sc_time_stamp() { return main_time; }

enum TestbenchState {
  starting,
  assert_reset,
  deassert_reset,
  wait_until_rdy_is_low,
  write_inputs,
  assert_ena,
  wait_until_rdy_is_high,
  deassert_ena,
  read_outputs,
  finished,
};

int main(int argc, char **argv) {
  std::cout << "hello from " << __FILE__ << std::endl;
  const std::unique_ptr<VerilatedContext> contextp{new VerilatedContext};
  contextp->commandArgs(argc, argv);
  const std::unique_ptr<Vtop> top{new Vtop{contextp.get(), "TOP"}};
#if VM_TRACE
  // Enable tracing
  VerilatedVcdC *tfp = NULL;
  const char *flag = Verilated::commandArgsPlusMatch("trace");
  if (flag && 0 == strcmp(flag, "+trace")) {
    Verilated::traceEverOn(true);
    tfp = new VerilatedVcdC;
    top->trace(tfp, 99);
    Verilated::mkdir("logs");
    tfp->open("logs/vlt_dump.vcd");
  }
#endif

  // FIXME: miksi ei suorita kaikkia testcaseja??

  // Testbench data
  std::fstream fs("testcases.txt", std::ios_base::in);
  if (!fs) {
    printf("testcases.txt not found\n");
    return -1;
  }
  std::vector<std::vector<int>> data;
  std::string tmp;
  while (std::getline(fs, tmp)) {
    std::istringstream iss(tmp);
    std::vector<std::string> tokens;
    {
      std::string token;
      while (std::getline(iss, token, ' ')) {
        tokens.push_back(token);
      }
    }
    std::vector<int> values;
    for (const auto &token : tokens) {
      try {
        int value = std::stoi(token);
        values.push_back(value);
      } catch (const std::invalid_argument &e) {
        std::cerr << "invalid argument: " << e.what() << std::endl;
        return -1;
      } catch (const std::out_of_range &e) {
        std::cerr << "out of range: " << e.what() << std::endl;
        return -1;
      }
    }
    data.push_back(values);
  }
  std::cout << "found"
            << " " << data.size() << " "
            << "testcases" << std::endl;
  const int maximum_iterations = 10000;
  TestbenchState tb_state = TestbenchState::starting;
  std::vector<std::vector<int>>::iterator row_it = data.begin();
  int data_index = 0;
  for (int i = 0; i < maximum_iterations; i++) {
    bool stop_iteration = false;
    switch (tb_state) {
    case TestbenchState::starting:
      printf("[%ld] initialize inputs\n", contextp->time());
      top->clk = 1;
      top->rst = 0;
      top->in1 = 0;
      top->in2 = 0;
      top->ena = 0;
      tb_state = TestbenchState::assert_reset;
      break;
    case TestbenchState::assert_reset:
      printf("[%ld] assert reset\n", contextp->time());
      top->rst = 1;
      tb_state = TestbenchState::deassert_reset;
      break;
    case TestbenchState::deassert_reset:
      printf("[%ld] deassert reset\n", contextp->time());
      top->rst = 0;
      tb_state = TestbenchState::wait_until_rdy_is_low;
      break;
    case TestbenchState::wait_until_rdy_is_low:
      printf("[%ld] wait until rdy is low\n", contextp->time());
      if (top->rdy == 0) {
        tb_state = TestbenchState::write_inputs;
      } else {
        tb_state = TestbenchState::wait_until_rdy_is_low;
      }
      break;
    case TestbenchState::write_inputs:
      printf("[%ld] write inputs\n", contextp->time());
      data_index = std::distance(data.begin(), row_it);
      printf("[%ld] data index %d\n", contextp->time(), data_index);
      top->in1 = (*row_it)[0];
      top->in2 = (*row_it)[1];
      tb_state = TestbenchState::assert_ena;
      break;
    case TestbenchState::assert_ena:
      printf("[%ld] assert ena\n", contextp->time());
      top->ena = 1;
      tb_state = TestbenchState::wait_until_rdy_is_high;
      break;
    case TestbenchState::wait_until_rdy_is_high:
      printf("[%ld] wait until rdy is high\n", contextp->time());
      if (top->rdy == 1) {
        // printf("[%d] received high rdy\n", contextp->time());
        tb_state = TestbenchState::deassert_ena;
      } else {
        tb_state = TestbenchState::wait_until_rdy_is_high;
      }
      break;
    case TestbenchState::deassert_ena:
      printf("[%ld] deassert ena\n", contextp->time());
      top->ena = 0;
      tb_state = TestbenchState::read_outputs;
      break;
    case TestbenchState::read_outputs:
      printf("[%ld] read outputs\n", contextp->time());
      if (top->out == (*row_it)[2]) {
        printf("[%ld] ok %d %d %d %d %d %d %d\n", contextp->time(), top->clk,
               top->rst, top->in1, top->in2, top->ena, top->out, top->rdy);
      } else {
        printf("[%ld] fail %d %d %d %d %d %d %d\n", contextp->time(), top->clk,
               top->rst, top->in1, top->in2, top->ena, top->out, top->rdy);
        data_index = std::distance(data.begin(), row_it);
        printf("[%ld] fail @ data_index=%d: top->out %d != expected %d\n",
               contextp->time(), data_index, top->out, (*row_it)[2]);
      }
      row_it = std::next(row_it);
      if (row_it == data.end()) {
        printf("[%ld] all test data consumed\n", contextp->time());
        tb_state = TestbenchState::finished;
      } else {
        tb_state = TestbenchState::wait_until_rdy_is_low;
      }
      break;
    case TestbenchState::finished:
      printf("[%ld] finished\n", contextp->time());
      stop_iteration = true;
      tb_state = TestbenchState::finished;
      break;
    }
    main_time++;
    top->clk = !top->clk;
    top->eval();
#if VM_TRACE
    if (tfp) {
      tfp->dump(main_time);
    }
#endif
    if (stop_iteration) {
      break;
    }
  }
  printf("[final] %d %d %d %d %d %d %d\n", top->clk, top->rst, top->in1,
         top->in2, top->ena, top->out, top->rdy);
  if (row_it != data.end()) {
    printf("[%ld] fail not all test data consumed\n", contextp->time());
  }
#if VM_TRACE
  if (tfp) {
    tfp->dump(main_time);
  }
#endif
  top->final();
#if VM_TRACE
  if (tfp) {
    tfp->close();
    tfp = NULL;
  }
#endif
  return 0;
}
