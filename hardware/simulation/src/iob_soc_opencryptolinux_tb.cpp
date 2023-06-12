#include <stdio.h>
#include <stdlib.h>

#include "Viob_soc_opencryptolinux_sim_wrapper.h"
#include "bsp.h"
#include "verilated.h"
#include "verilated_vcd_c.h"

#include "iob_soc_opencryptolinux_conf.h"

// other macros
#define CLK_PERIOD 1000000000 / FREQ // 1/100MHz*10^9 = 10 ns

vluint64_t main_time = 0;
VerilatedVcdC *tfp = NULL;
Viob_soc_opencryptolinux_sim_wrapper *dut = NULL;

double sc_time_stamp() { return main_time; }

void Timer(unsigned int ns) {
  for (int i = 0; i < ns; i++) {
    if (!(main_time % (CLK_PERIOD / 2))) {
      dut->clk_i = !(dut->clk_i);
      dut->eval();
    }
    // To add a new clk follow the example
    // if(!(main_time%(EXAMPLE_CLK_PERIOD/2))){
    //  dut->example_clk_in = !(dut->example_clk_in);
    //}
    main_time += 1;
  }
}

// 1-cycle write
void uartwrite(unsigned int cpu_address, unsigned int cpu_data,
               unsigned int nbytes) {
  char wstrb_int = 0;
  switch (nbytes) {
  case 1:
    wstrb_int = 0b01;
    break;
  case 2:
    wstrb_int = 0b011;
    break;
  default:
    wstrb_int = 0b01111;
    break;
  }
  dut->uart_addr = cpu_address;
  dut->uart_avalid = 1;
  dut->uart_wstrb = wstrb_int << (cpu_address & 0b011);
  dut->uart_wdata = cpu_data
                    << ((cpu_address & 0b011) * 8); // align data to 32 bits
  Timer(CLK_PERIOD);
  dut->uart_wstrb = 0;
  dut->uart_avalid = 0;
}

// 2-cycle read
void uartread(unsigned int cpu_address, unsigned char *read_reg) {
  dut->uart_addr = cpu_address;
  dut->uart_avalid = 1;
  Timer(CLK_PERIOD);
  *read_reg =
      (dut->uart_rdata) >> ((cpu_address & 0b011) * 8); // align to 32 bits
  dut->uart_avalid = 0;
}

void inituart() {
  // Set the Line Control Register to the desired line control parameters.
  // Set bit 7 to ‘1’ to allow access to the Divisor Latches.
  uint8_t lcr = 0;
  uartread(3, &lcr);
  lcr = (lcr|128);
  uartwrite(3, lcr, 1);
  // Set the Divisor Latches, MSB first, LSB next.
  uint16_t div = FREQ/BAUD;
  uint8_t *dl = (uint8_t *)&div;
  uartwrite(1, *(dl+1), 1);
  uartwrite(0, *(dl), 1);
  // Set bit 7 of LCR to ‘0’ to disable access to Divisor Latches.
  // At this time the transmission engine starts working and data can be sent and received.
  lcr = (lcr&127);
  uartwrite(3, lcr, 1);
  // Set the FIFO trigger level. Generally, higher trigger level values produce less
  // interrupt to the system, so setting it to 14 bytes is recommended if the system
  // responds fast enough.
  uartwrite(2, 192, 1);
  // Enable desired interrupts by setting appropriate bits in the Interrupt Enable
  // register.
  uartwrite(1, 3, 1);
}

void write_success(){
  FILE* fd = 0;
  fd = fopen("test.log", "w");
  fprintf(fd, "Test passed!");
  fclose(fd);
}

int main(int argc, char **argv, char **env) {
  Verilated::commandArgs(argc, argv);
  Verilated::traceEverOn(true);
  dut = new Viob_soc_opencryptolinux_sim_wrapper;

#ifdef VCD
  tfp = new VerilatedVcdC;

  dut->trace(tfp, 1);
  tfp->open("uut.vcd");
#endif

  dut->clk_i = 0;
  dut->rst_i = 0;

  // Reset sequence
  Timer(100);
  dut->rst_i = 1;
  Timer(100);
  dut->rst_i = 0;

  dut->uart_avalid = 0;
  dut->uart_wstrb = 0;
  inituart();

  FILE *soc2cnsl_fd;
  FILE *cnsl2soc_fd;
  unsigned char cpu_char = 0;
  unsigned char rxread_reg = 0, txread_reg = 0;
  unsigned char rxread_aux = 0, txread_aux = 0;
  int able2write = 0, able2read = 0;

  while ((cnsl2soc_fd = fopen("./cnsl2soc", "rb")) == NULL)
    ;
  fclose(cnsl2soc_fd);

  while (1) {
    if (dut->trap_o > 0) {
      printf("\nTESTBENCH: force cpu trap exit\n");
      soc2cnsl_fd = fopen("./soc2cnsl", "wb");
      cpu_char = 4;
      fwrite(&cpu_char, sizeof(char), 1, soc2cnsl_fd);
      fclose(soc2cnsl_fd);
      break;
    }
    while (!rxread_aux && !txread_aux) {
      uartread(5, &rxread_aux);
      uartread(5, &txread_aux);
      rxread_reg = rxread_aux & (1<<6);
      txread_reg = txread_aux & 1;
    }
    if (rxread_reg) {
      if ((soc2cnsl_fd = fopen("./soc2cnsl", "rb")) != NULL) {
        able2read = fread(&cpu_char, sizeof(char), 1, soc2cnsl_fd);
        if (able2read == 0) {
          fclose(soc2cnsl_fd);
          uartread(0, &cpu_char);
          soc2cnsl_fd = fopen("./soc2cnsl", "wb");
          fwrite(&cpu_char, sizeof(char), 1, soc2cnsl_fd);
          rxread_reg = 0;
        }
        fclose(soc2cnsl_fd);
      }
    }
    if (txread_reg) {
      if ((cnsl2soc_fd = fopen("./cnsl2soc", "rb")) == NULL) {
        break;
      }
      able2write = fread(&cpu_char, sizeof(char), 1, cnsl2soc_fd);
      if (able2write > 0) {
        uartwrite(0, cpu_char, 1);
        fclose(cnsl2soc_fd);
        cnsl2soc_fd = fopen("./cnsl2soc", "w");
      }
      fclose(cnsl2soc_fd);
      txread_reg = 0;
    }
  }

  dut->final();
#ifdef VCD
  tfp->close();
#endif
  delete dut;
  dut = NULL;
  exit(0);
}
