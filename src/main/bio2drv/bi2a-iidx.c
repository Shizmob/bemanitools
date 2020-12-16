#define LOG_MODULE "bio2drv-bi2a_iidx"

#include "bio2drv/bi2a-iidx.h"

#include <stdio.h>
#include <string.h>

#include "aciodrv/device.h"

#include "util/log.h"

static bool bio2drv_bi2a_iidx_init_io(uint8_t node_id)
{
    struct ac_io_message msg;

    msg.addr = node_id + 1;
    msg.cmd.code = ac_io_u16(AC_IO_CMD_CLEAR);
    msg.cmd.nbytes = 1;
    msg.cmd.count = 0x3B;

    if (!aciodrv_send_and_recv(
            &msg, offsetof(struct ac_io_message, cmd.raw) + 1)) {
        log_warning("Init node failed");
        return 0;
    }

    log_warning("Init of node %d, status: %d", node_id, msg.cmd.status);

    return 1;
}

static bool bio2drv_bi2a_iidx_watchdog_start(uint8_t node_id)
{
    // exit early and don't actually call watchdog
    // the watchdog call actually returns different sized packets depending on
    // the state this results in an issue during packet processing (see: #68)
    return true;

    /*
    struct ac_io_message msg;

    msg.addr = node_id + 1;
    msg.cmd.code = ac_io_u16(AC_IO_CMD_KFCA_WATCHDOG);
    msg.cmd.nbytes = 2;
    msg.cmd.nbytes = 2;

    // uint16_t: 6000
    msg.cmd.raw[0] = 23;
    msg.cmd.raw[1] = 112;

    if (!aciodrv_send_and_recv(
        &msg, offsetof(struct ac_io_message, cmd.raw) + 2
    )) {
        log_warning("Starting watchdog failed"); return false;
    }

    log_warning("Started watchdog of node %d, status: %d",
        node_id, msg.cmd.status);

    return true;
    */
}

bool bio2drv_bi2a_iidx_init(uint8_t node_id)
{
    if (!bio2drv_bi2a_iidx_init_io(node_id)) {
        return false;
    }

    if (!bio2drv_bi2a_iidx_watchdog_start(node_id)) {
        return false;
    }

    return true;
}

bool bio2drv_bi2a_iidx_poll(
    uint8_t node_id,
    const struct bi2a_iidx_state_out *pout,
    struct bi2a_iidx_state_in *pin)
{
    struct ac_io_message msg;

    msg.addr = node_id + 1;
    msg.cmd.code = ac_io_u16(BIO2_BI2A_CMD_POLL);
    msg.cmd.nbytes = sizeof(*pout);
    /* buffer size of data we expect */
    *(struct bi2a_iidx_state_out *) msg.cmd.raw = *pout;

    if (!aciodrv_send_and_recv(
            &msg, offsetof(struct ac_io_message, cmd.raw) + sizeof(*pin))) {
        log_warning("Polling of node %d failed", node_id + 1);
        return false;
    }

    if (pin != NULL) {
        memcpy(pin, &msg.cmd.raw, sizeof(*pin));
    }

    return true;
}