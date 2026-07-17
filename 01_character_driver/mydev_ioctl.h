#ifndef MYDEV_IOCTL_H
#define MYDEV_IOCTL_H

#include <linux/ioctl.h>

/* ── ioctl magic number ──
 * Unique identifier for your driver
 * Prevents conflicts with other drivers
 * Convention: pick a letter not used by others
 * Full list: Documentation/userspace-api/ioctl/ioctl-number.rst */
#define MYDEV_MAGIC  'M'

/* ── command definitions ──
 * _IO   (magic, nr)        → no data transfer
 * _IOR  (magic, nr, type)  → read FROM driver (driver→userspace)
 * _IOW  (magic, nr, type)  → write TO driver  (userspace→driver)
 * _IOWR (magic, nr, type)  → both directions  */

/* Clear kernel buffer */
#define MYDEV_CLEAR     _IO(MYDEV_MAGIC,  0)

/* Get current buffer length → returns int to userspace */
#define MYDEV_GET_LEN   _IOR(MYDEV_MAGIC, 1, int)

/* Set buffer size → userspace sends int to driver */
#define MYDEV_SET_SIZE  _IOW(MYDEV_MAGIC, 2, int)

/* Get driver stats → userspace sends struct, driver fills it */
typedef struct {
    int buf_size;      /* current buffer size */
    int buf_len;       /* bytes currently stored */
    int read_count;    /* total read() calls */
    int write_count;   /* total write() calls */
} mydev_stats_t;

#define MYDEV_GET_STATS _IOR(MYDEV_MAGIC, 3, mydev_stats_t)

#endif /* MYDEV_IOCTL_H */
