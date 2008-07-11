/*
 * File:  logicrcv.h - Logical recovery
 * Copyright (C) 2008 The Institute for System Programming of the Russian Academy of Sciences (ISP RAS)
 *
 * Only the user interface is specified here. For further information refer to logicrcv.cpp file.
 *
 */

#ifndef _LL_LOGICAL_RCV_
#define _LL_LOGICAL_RCV_

#include "sm/llsm/llMain.h"

// Resores logical state of the database
// Parameters:
// 		start_lsn - where to start looking for logical records
void llLogicalRecover(const LSN start_lsn);

// Rolls back specified transaction
// Parameters:
// 		trid - transaction identifier
void llLogRollbackTrn(transaction_id trid);

#ifdef SE_ENABLE_FTSEARCH
// this function performs remapping on ft-indexes (needed since we use redo-remapping)
void rcvRecoverFtIndexes();
#endif

#endif
