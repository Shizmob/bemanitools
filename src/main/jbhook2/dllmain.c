#include <windows.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "bemanitools/eamio.h"
#include "bemanitools/jbio.h"

#include "hook/iohook.h"
#include "hook/table.h"

#include "hooklib/adapter.h"
#include "hooklib/app.h"
#include "hooklib/rs232.h"
#include "hooklib/setupapi.h"

#include "imports/avs.h"

#include "jbhook2/options.h"

#include "jbhook-util/acio.h"
#include "jbhook-util/eamuse.h"
#include "jbhook-util/gfx.h"
#include "jbhook-util/p3io.h"
#include "jbhook-util/security.h"

#include "p3ioemu/devmgr.h"
#include "p3ioemu/emu.h"

#include "p4ioemu/device.h"
#include "p4ioemu/setupapi.h"

#include "security/id.h"

#include "util/defs.h"
#include "util/log.h"
#include "util/thread.h"

static struct options options;

static bool my_dll_entry_init(char *sidcode, struct property_node *param)
{
    jbhook_util_gfx_install_vertical_hooks();

    bool eam_io_ok;
    bool jb_io_ok;

    eam_io_ok = false;
    jb_io_ok = false;

    log_info("--- Begin jbhook dll_entry_init ---");

    if (!options.disable_p3ioemu) {
        log_assert(sidcode != NULL);
        // pcbid and eamid are only used here for sec check, the ones used for
        // network access are taken from ea3-config.xml
        jbhook_util_p3io_init(
            &jbhook_util_security_default_mcode,
            &security_id_default, &security_id_default);

        log_info("Starting up jubeat IO backend");

        jb_io_set_loggers(
            log_impl_misc, log_impl_info, log_impl_warning, log_impl_fatal);

        jb_io_ok =
            jb_io_init(avs_thread_create, avs_thread_join, avs_thread_destroy);

        if (!jb_io_ok) {
            goto fail;
        }
    }

    if (!options.disable_cardemu) {
        jbhook_util_ac_io_port_init(L"COM1");

        log_info("Starting up card reader backend");

        eam_io_set_loggers(
            log_impl_misc, log_impl_info, log_impl_warning, log_impl_fatal);

        eam_io_ok =
            eam_io_init(avs_thread_create, avs_thread_join, avs_thread_destroy);

        if (!eam_io_ok) {
            goto fail;
        }
    }

    log_info("---  End  jbhook dll_entry_init ---");

    bool ret = app_hook_invoke_init(sidcode, param);

    return ret;

fail:
    if (eam_io_ok) {
        eam_io_fini();
    }

    if (jb_io_ok) {
        jb_io_fini();
    }

    return false;
}

static bool my_dll_entry_main(void)
{
    bool result;

    result = app_hook_invoke_main();

    log_info("Shutting down card reader backend");
    eam_io_fini();

    log_info("Shutting down Jubeat IO backend");
    jb_io_fini();

    if (!options.disable_cardemu) {
        jbhook_util_ac_io_port_fini();
    }

    if (!options.disable_p3ioemu) {
        jbhook_util_p3io_fini();
    }

    options_fini(&options);

    return result;
}

static HWND CDECL
my_mwindow_create(HINSTANCE, void *, const char *, DWORD, DWORD, BOOL);
static HWND(CDECL *real_mwindow_create)(
    HINSTANCE, void *, const char *, DWORD, DWORD, BOOL);

static const struct hook_symbol init_hook_syms[] = {
    {
        .name = "mwindow_create",
        .patch = my_mwindow_create,
        .link = (void **) &real_mwindow_create,
    },
};

static HWND CDECL my_mwindow_create(
    HINSTANCE hinstance,
    void *callback,
    const char *window_title,
    DWORD window_width,
    DWORD window_height,
    BOOL fullscreen)
{
    log_info("-------------------------------------------------------------");
    log_info("---------------- Begin jbhook mwindow_create ----------------");
    log_info("-------------------------------------------------------------");

    DWORD tmp = window_width;
    window_width = window_height;
    window_height = tmp;

    return real_mwindow_create(
        hinstance,
        callback,
        window_title,
        window_width,
        window_height,
        0);
}

BOOL WINAPI DllMain(HMODULE mod, DWORD reason, void *ctx)
{
    if (reason != DLL_PROCESS_ATTACH) {
        return TRUE;
    }

    log_to_external(
        log_body_misc, log_body_info, log_body_warning, log_body_fatal);

    options_init_from_cmdline(&options);

        hook_table_apply(
            NULL, "mwindow.dll", init_hook_syms, lengthof(init_hook_syms));

    app_hook_init(my_dll_entry_init, my_dll_entry_main);

    iohook_push_handler(p3io_emu_dispatch_irp);

    iohook_push_handler(jbhook_util_ac_io_port_dispatch_irp);

    if (!options.disable_adapteremu) {
        adapter_hook_init();
    }

    if (!options.disable_cardemu) {
        rs232_hook_init();
    }

    if (!options.disable_p3ioemu) {
        p3io_setupapi_insert_hooks(NULL);
    }

    jbhook_util_gfx_hook_init();
    jbhook_util_eamuse_hook_init();

    return TRUE;
}