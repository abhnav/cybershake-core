#include "mpi.h"
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/file.h>
#include <sys/resource.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/syscall.h>

#include "defs.h"
#include "defs_infc.h"

#include "structure.h"
#include "function.h"
