#define LOG_MODULE "aciodrv-kfca"

#include <stdio.h>
#include <string.h>

#include "aciodrv/device.h"

#include "util/log.h"

static bool aciodrv_kfca_watchdog_start(uint8_t node_id)
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

    if (!aciodrv_send_and_recv(&msg, offsetof(struct ac_io_message, cmd.raw) +
    2)) { log_warning("Starting watchdog failed"); return false;
    }

    log_warning("Started watchdog of node %d, status: %d",
        node_id, msg.cmd.status);

    return true;
    */
}

bool aciodrv_kfca_amp(
    uint8_t node_id,
    uint8_t primary,
    uint8_t headphone,
    uint8_t unused,
    uint8_t subwoofer)
{
    struct ac_io_message msg;

    msg.addr = node_id + 1;
    msg.cmd.code = ac_io_u16(AC_IO_CMD_KFCA_AMP_CONTROL);
    msg.cmd.nbytes = 4;

    msg.cmd.raw[0] = primary;
    msg.cmd.raw[1] = headphone;
    msg.cmd.raw[2] = unused;
    msg.cmd.raw[3] = subwoofer;

    if (!aciodrv_send_and_recv(
            &msg, offsetof(struct ac_io_message, cmd.raw) + 1)) {
        log_warning("Setting AMP failed");
        return false;
    }

    log_warning("Started AMP node %d", node_id);

    return true;
}

bool aciodrv_kfca_init(uint8_t node_id)
{
    if (!aciodrv_kfca_watchdog_start(node_id)) {
        return false;
    }

    if (!aciodrv_kfca_amp(node_id, 0, 0, 0, 0)) {
        return false;
    }

    return true;
}

bool aciodrv_kfca_poll(
    uint8_t node_id,
    const struct ac_io_kfca_poll_out *pout,
    struct ac_io_kfca_poll_in *pin)
{
    struct ac_io_message msg;

    msg.addr = node_id + 1;
    msg.cmd.code = ac_io_u16(AC_IO_CMD_KFCA_POLL);
    msg.cmd.nbytes = sizeof(*pout);
    /* buffer size of data we expect */
    msg.cmd.kfca_poll_out = *pout;

    if (!aciodrv_send_and_recv(
            &msg, offsetof(struct ac_io_message, cmd.raw) + sizeof(*pin))) {
        log_warning("Polling of node %d failed", node_id + 1);
        return false;
    }

    if (pin != NULL) {
        memcpy(pin, &msg.cmd.kfca_poll_in, sizeof(*pin));
    }

    return true;
}
