#include <common.h>

#include <init.h>
#include <asm/global_data.h>
#include <asm/arch/imx-regs.h>


DECLARE_GLOBAL_DATA_PTR;

int dram_init(void) {
    /* dram_init must store complete ramsize in gd->ram_size */
    gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE, PHYS_SDRAM_1_SIZE);
    return 0;
}

int board_init(void) {
    return 0;
}

// TODO: here !

int board_early_init_f(void)
{
	// setup_iomux_uart();

	return 0;
}
#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{


	return 0;
}
#endif
