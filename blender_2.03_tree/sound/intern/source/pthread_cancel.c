/* $Id: pthread_cancel.c,v 1.1 2000/08/02 13:42:08 hans Exp $
 * FreeBSD 3.4 does not yet have pthread_cancel (3.5 and above do)
 */
#ifdef __FreeBSD__
#if (__FreeBSD_version < 350000)
#include <pthread.h>
int pthread_cancel(pthread_t pthread) {
    pthread_exit(NULL);
}
#endif
#endif
