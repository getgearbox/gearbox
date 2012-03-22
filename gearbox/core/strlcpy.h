#ifndef GEARBOX_STRLCPY_H
#define GEARBOX_STRLCPY_H

#include <sys/types.h>
#include <string.h>

namespace Gearbox {
/**
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
static inline size_t
strlcpy(char *dst, const char *src, size_t siz)
{
    size_t ret = strlen(src);
    if (!siz) return ret;

    size_t to_copy = (siz - 1) > ret ? ret : (siz - 1);
    memcpy(dst, src, to_copy);
    dst[to_copy] = '\0';

    return ret;
}

} // namespace

#endif // GEARBOX_STRLCPY_H
