/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
/*
 * This file is part of libaacs
 * Copyright (C) 2009-2010  Obliter0n
 * Copyright (C) 2010-2015  npzacs
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "mmc_device.h"

#include "path.h"

#include "util/logging.h"
#include "util/macro.h"
#include "util/strutl.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifdef HAVE_MNTENT_H
#include <mntent.h>
#endif

#ifdef HAVE_LINUX_CDROM_H
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/cdrom.h>
#endif


/*
 *
 */

struct mmcdev {
    int fd;
};

int device_send_cmd(MMCDEV *dev, const uint8_t *cmd, uint8_t *buf, size_t tx, size_t rx)
{
#if defined(HAVE_LINUX_CDROM_H)
    struct cdrom_generic_command cgc;
    struct request_sense sense;
    char str[512];
    int  result;

    memset(&cgc, 0, sizeof(cgc));
    memcpy(cgc.cmd, cmd, CDROM_PACKET_SIZE);
    cgc.sense = &sense;
    cgc.timeout = 5000;

    if (buf) {
        if (tx) {
            cgc.data_direction = CGC_DATA_WRITE;
            cgc.buflen = tx;
            cgc.buffer = buf;
        } else if (rx) {
            cgc.data_direction = CGC_DATA_READ;
            cgc.buflen = rx;
            cgc.buffer = buf;
        }
    } else {
        cgc.data_direction = CGC_DATA_NONE;
        cgc.buflen = 0;
        cgc.buffer = NULL;
    }

    result = ioctl(dev->fd, CDROM_SEND_PACKET, &cgc);

    BD_DEBUG(DBG_MMC, "Send LINUX MMC cmd %s:\n", str_print_hex(str, cmd, 16));
    if (buf) {
    if (tx) {
        BD_DEBUG(DBG_MMC, "  Buffer: %s ->\n", str_print_hex(str, buf, tx>255?255:tx));
    } else {
        BD_DEBUG(DBG_MMC, "  Buffer: %s <-\n", str_print_hex(str, buf, rx>255?255:rx));
    }
    }

    if (result >= 0) {
        BD_DEBUG(DBG_MMC, "  Send succeeded! [%d]\n", result);
        return 1;
    }

    BD_DEBUG(DBG_MMC, "  Send failed! [%d] %s\n", result, strerror(errno));
#else
#warning no MMC drive support
    (void)dev; (void)cmd; (void)buf; (void)tx; (void)rx;
    BD_DEBUG(DBG_MMC | DBG_CRIT, "No MMC drive support\n");
#endif

    return 0;
}

static int _open_block_device(const char *path)
{
    struct stat st;
    int         fd;

    fd = open(path, O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        BD_DEBUG(DBG_MMC | DBG_CRIT, "Error opening %s\n", path);
        return -1;
    }

    if (!fstat(fd, &st) && S_ISBLK(st.st_mode)) {
        BD_DEBUG(DBG_MMC, "Opened block device %s\n", path);
        return fd;
    }

    BD_DEBUG(DBG_MMC, "%s is not a block device\n", path);
    close(fd);
    return -1;
}

MMCDEV *device_open(const char *path)
{
    char       *resolved_path;
    size_t      path_len;
    int         fd;

    /* resolve path */
    resolved_path = aacs_resolve_path(path);
    if (!resolved_path) {
        BD_DEBUG(DBG_MMC | DBG_CRIT, "Failed resolving path %s\n", path);
        return NULL;
    }

    /* strip trailing '/'s */
    path_len = strlen(resolved_path);
    while (path_len > 0 && resolved_path[--path_len] == '/') {
        resolved_path[path_len] = '\0';
    }

    fd = _open_block_device(resolved_path);
    if (fd < 0) {
#if defined(HAVE_MNTENT_H)
        /* resolve mount point to block device */
        FILE *proc_mounts;
        if ((proc_mounts = setmntent("/proc/mounts", "r"))) {
            struct mntent* mount_entry;

            BD_DEBUG(DBG_MMC, "Opening LINUX MMC drive mounted to %s...\n", resolved_path);
            while ((mount_entry = getmntent(proc_mounts)) != NULL) {
                if (strcmp(mount_entry->mnt_dir, resolved_path) == 0) {
                    fd = open(mount_entry->mnt_fsname, O_RDONLY | O_NONBLOCK);
                    if (fd >= 0) {
                        BD_DEBUG(DBG_MMC, "LINUX MMC drive %s opened - fd: %d\n",
                              mount_entry->mnt_fsname, fd);
                        break;
                    }

                    BD_DEBUG(DBG_MMC | DBG_CRIT, "Failed opening MMC drive %s mounted to %s\n",
                          mount_entry->mnt_fsname, resolved_path);
                }
            }

            endmntent(proc_mounts);

            if (fd < 0) {
                BD_DEBUG(DBG_MMC | DBG_CRIT, "No MMC drive mounted to %s\n", resolved_path);
            }

        } else {
            BD_DEBUG(DBG_MMC | DBG_CRIT, "Error opening /proc/mounts\n");
        }
#else
        BD_DEBUG(DBG_MMC | DBG_CRIT, "Only block devices supported\n");
        X_FREE(resolved_path);
        return NULL;
#endif
    }

    X_FREE(resolved_path);

    if (fd >= 0) {
        MMCDEV *dev = calloc(1, sizeof(MMCDEV));
        if (dev) {
            dev->fd = fd;
            return dev;
        }
        close(fd);
    }

    return NULL;
}

void device_close(MMCDEV **pp)
{
    if (pp && *pp) {

        if ((*pp)->fd != -1) {
            close((*pp)->fd);
        }

        X_FREE(*pp);
    }
}
