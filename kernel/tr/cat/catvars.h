#ifndef CATVARS_H
#define CATVARS_H

#include "common/xptr.h"

/* Catalog cache parameters */

extern const bool ccache_available;
extern int ccache_size;

/* Last root NID */

extern int last_nid_size;
extern unsigned char * last_nid;

/* Pointer to catalog masterdata block */

extern xptr catalog_masterblock;

#endif 
