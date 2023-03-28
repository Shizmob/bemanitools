#define LOG_MODULE "ezusb-emu-node-sec-mem"

#include <string.h>

#include "ezusb-emu/node-security-mem.h"
#include "ezusb-iidx/secmem-cmd.h"

#include "util/log.h"

#define SECURITY2_NPAGES 5

/* Starting GOLD with the new IO2 (C02 is not affected), the game does not
   transfer 5 pages of security data to the IO2 board. Instead, the board seems
   to be preloaded with security data by the firmware (?) and just a single
   page of data is sent to the board to store. The following dump was taken
   from the GOLD exec 2007022101
*/
static uint8_t
    ezusb_iidx_emu_node_security_mem_mem[EZUSB_PAGESIZE * SECURITY2_NPAGES] = {
        0x00, 0x90, 0x31, 0xCF, 0x95, 0x7A, 0x59, 0xE5, 0xD2, 0xBF, 0x2C, 0xDB,
        0xB5, 0x83, 0x4D, 0x03, 0x17, 0x5D, 0x25, 0x2A, 0xFD, 0x72, 0x1E, 0x01,
        0x02, 0x60, 0x88, 0x92, 0x9A, 0x9B, 0x2A, 0xA9, 0x73, 0x5A, 0x0E, 0x9B,
        0xC8, 0xCD, 0x85, 0x4D, 0xE0, 0xBA, 0xF4, 0xEC, 0x8A, 0x24, 0x76, 0x3C,
        0xDC, 0x35, 0xC7, 0xD7, 0xFF, 0xFF, 0x9C, 0x64, 0x44, 0x4C, 0xD7, 0x06,
        0x60, 0x17, 0xAD, 0x0E, 0x02, 0xEB, 0x46, 0x45, 0x96, 0xB0, 0xD6, 0xB9,
        0x7C, 0x34, 0xBE, 0x77, 0x75, 0xF2, 0xBE, 0x1B, 0x99, 0x62, 0xBC, 0x9B,
        0x92, 0x5C, 0x26, 0x39, 0x6C, 0xCD, 0x84, 0xFD, 0xC0, 0x58, 0x2B, 0xA8,
        0x7D, 0x10, 0xB3, 0x81, 0x25, 0xF3, 0x24, 0xE7, 0xB1, 0x4D, 0x6D, 0x12,
        0xF7, 0xAE, 0x27, 0xE0, 0xD2, 0x95, 0x30, 0x2D, 0xD1, 0x79, 0x27, 0x81,
        0xBB, 0x67, 0x47, 0x91, 0xAE, 0xC1, 0xB8, 0x79, 0x1F, 0x5E, 0xD5, 0x08,
        0x84, 0xA9, 0x6D, 0x1A, 0xF3, 0xEB, 0x8C, 0x58, 0x78, 0x5F, 0xD8, 0x51,
        0x74, 0x45, 0xFB, 0x4C, 0xBD, 0x91, 0x32, 0xC2, 0xD6, 0x65, 0x80, 0xE3,
        0x07, 0xFE, 0x92, 0x0C, 0x88, 0x31, 0xD7, 0xA0, 0xA8, 0x32, 0xD7, 0x1F,
        0x1C, 0xBE, 0x50, 0xF0, 0x49, 0x56, 0x23, 0xBB, 0xD5, 0xB5, 0x99, 0xBF,
        0x40, 0x24, 0x00, 0x0F, 0xCE, 0xDA, 0x35, 0x1D, 0x8D, 0x03, 0x1D, 0x74,
        0xC0, 0xAF, 0x8B, 0x12, 0x6F, 0x33, 0xB2, 0x4A, 0x6F, 0x3B, 0x93, 0x88,
        0xA0, 0x29, 0x81, 0xF6, 0xB2, 0xEC, 0x30, 0x56, 0x2D, 0xFE, 0x75, 0xFF,
        0x18, 0xA0, 0x18, 0x70, 0xEE, 0x0C, 0xE5, 0x4A, 0x3A, 0xC4, 0x69, 0x33,
        0xA0, 0x9A, 0x73, 0x77, 0x99, 0xA2, 0xDA, 0xD4, 0x9F, 0xB8, 0x90, 0x60,
        0x2F, 0xBC, 0x8E, 0xE7, 0x3E, 0x30, 0x9A, 0xB2, 0x95, 0x59, 0x7E, 0x14,
        0xBD, 0x9C, 0x9E, 0xB0};

void ezusb_iidx_emu_node_security_mem_init(void)
{
    /* Starting GOLD with the IO2 board, we have to preload the memory.
       Doesn't matter for the C02 board overwriting the memory anyway */
    /*
    memset(ezusb_iidx_emu_node_security_mem_mem, 0xFF,
        sizeof(ezusb_iidx_emu_node_security_mem_mem));
    */
}

uint8_t ezusb_iidx_emu_node_security_mem_v1_process_cmd(
    uint8_t cmd_id, uint8_t cmd_data, uint8_t cmd_data2)
{
    switch (cmd_id) {
        case EZUSB_IIDX_SECMEM_CMD_INIT:
            log_misc("EZUSB_SECURITY_MEM_V1_CMD_INIT");
            return EZUSB_IIDX_SECMEM_CMD_STATUS_V1_INIT_OK;

        default:
            log_warning(
                "Unrecognised security memory v1 command: %02x", cmd_id);
            return EZUSB_IIDX_SECMEM_CMD_STATUS_V1_FAULT;
    }
}

uint8_t ezusb_iidx_emu_node_security_mem_v2_process_cmd(
    uint8_t cmd_id, uint8_t cmd_data, uint8_t cmd_data2)
{
    switch (cmd_id) {
        case EZUSB_IIDX_SECMEM_CMD_INIT:
            log_misc("EZUSB_SECURITY_MEM_V2_CMD_INIT");
            return EZUSB_IIDX_SECMEM_CMD_STATUS_V2_INIT_OK;

        default:
            log_warning(
                "Unrecognised security memory v2 command: %02x", cmd_id);
            return EZUSB_IIDX_SECMEM_CMD_STATUS_V2_FAULT;
    }
}

bool ezusb_iidx_emu_node_security_mem_read_packet(
    struct ezusb_iidx_msg_bulk_packet *pkg)
{
    log_misc("Reading security memory");
    return true;
}

bool ezusb_iidx_emu_node_security_mem_write_packet(
    const struct ezusb_iidx_msg_bulk_packet *pkg)
{
    log_misc("Writing security memory 0x%02X", pkg->page);

    if (pkg->page >= SECURITY2_NPAGES) {
        log_warning("Writing security memory overrun");
        return false;
    }

    memcpy(
        ezusb_iidx_emu_node_security_mem_mem + pkg->page * EZUSB_PAGESIZE,
        pkg->payload,
        EZUSB_PAGESIZE);
    return true;
}

/* ------------------------------------------------------------------------- */

uint8_t ezusb_iidx_emu_node_security_mem_read_memory(uint32_t pos)
{
    if (pos < sizeof(ezusb_iidx_emu_node_security_mem_mem)) {
        return ezusb_iidx_emu_node_security_mem_mem[pos];
    }

    log_warning("Reading security memory overrun");
    return 0;
}