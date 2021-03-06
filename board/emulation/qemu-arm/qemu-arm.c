// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 Tuomas Tynkkynen
 */

#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <fdtdec.h>
#include <init.h>
#include <log.h>
#include <virtio_types.h>
#include <virtio.h>

#ifdef CONFIG_ARM64
#include <asm/armv8/mmu.h>

static struct mm_region qemu_arm64_mem_map[] = {
	{
		/* Flash */
		.virt = 0x00000000UL,
		.phys = 0x00000000UL,
		.size = 0x08000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/* Lowmem peripherals */
		.virt = 0x08000000UL,
		.phys = 0x08000000UL,
		.size = 0x38000000,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* RAM */
		.virt = 0x40000000UL,
		.phys = 0x40000000UL,
		.size = 255UL * SZ_1G,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE
	}, {
		/* Highmem PCI-E ECAM memory area */
		.virt = 0x4010000000ULL,
		.phys = 0x4010000000ULL,
		.size = 0x10000000,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* Highmem PCI-E MMIO memory area */
		.virt = 0x8000000000ULL,
		.phys = 0x8000000000ULL,
		.size = 0x8000000000ULL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN
	}, {
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = qemu_arm64_mem_map;
#endif

int board_init(void)
{
	return 0;
}

int board_late_init(void)
{
	/*
	 * Make sure virtio bus is enumerated so that peripherals
	 * on the virtio bus can be discovered by their drivers
	 */
	virtio_init();

	return 0;
}

int dram_init(void)
{
	if (fdtdec_setup_mem_size_base() != 0)
		return -EINVAL;

	return 0;
}

int dram_init_banksize(void)
{
	fdtdec_setup_memory_banksize();

	return 0;
}

void *board_fdt_blob_setup(int *err)
{
	*err = 0;
	/* QEMU loads a generated DTB for us at the start of RAM. */
	return (void *)CONFIG_SYS_SDRAM_BASE;
}

void enable_caches(void)
{
	 icache_enable();
	 dcache_enable();
}

#if defined(CONFIG_EFI_RNG_PROTOCOL)
#include <efi_loader.h>
#include <efi_rng.h>

#include <dm/device-internal.h>

efi_status_t platform_get_rng_device(struct udevice **dev)
{
	int ret;
	efi_status_t status = EFI_DEVICE_ERROR;
	struct udevice *bus, *devp;

	for (uclass_first_device(UCLASS_VIRTIO, &bus); bus;
	     uclass_next_device(&bus)) {
		for (device_find_first_child(bus, &devp); devp;
		     device_find_next_child(&devp)) {
			if (device_get_uclass_id(devp) == UCLASS_RNG) {
				*dev = devp;
				status = EFI_SUCCESS;
				break;
			}
		}
	}

	if (status != EFI_SUCCESS) {
		debug("No rng device found\n");
		return EFI_DEVICE_ERROR;
	}

	if (*dev) {
		ret = device_probe(*dev);
		if (ret)
			return EFI_DEVICE_ERROR;
	} else {
		debug("Couldn't get child device\n");
		return EFI_DEVICE_ERROR;
	}

	return EFI_SUCCESS;
}
#endif /* CONFIG_EFI_RNG_PROTOCOL */

#ifdef CONFIG_ARM64
#define __W	"w"
#else
#define __W
#endif

u8 flash_read8(void *addr)
{
	u8 ret;

	asm("ldrb %" __W "0, %1" : "=r"(ret) : "m"(*(u8 *)addr));
	return ret;
}

u16 flash_read16(void *addr)
{
	u16 ret;

	asm("ldrh %" __W "0, %1" : "=r"(ret) : "m"(*(u16 *)addr));
	return ret;
}

u32 flash_read32(void *addr)
{
	u32 ret;

	asm("ldr %" __W "0, %1" : "=r"(ret) : "m"(*(u32 *)addr));
	return ret;
}

void flash_write8(u8 value, void *addr)
{
	asm("strb %" __W "1, %0" : "=m"(*(u8 *)addr) : "r"(value));
}

void flash_write16(u16 value, void *addr)
{
	asm("strh %" __W "1, %0" : "=m"(*(u16 *)addr) : "r"(value));
}

void flash_write32(u32 value, void *addr)
{
	asm("str %" __W "1, %0" : "=m"(*(u32 *)addr) : "r"(value));
}
