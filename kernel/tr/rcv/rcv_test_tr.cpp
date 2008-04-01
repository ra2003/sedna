#include "tr/rcv/rcv_test_tr.h"
#include "common/xptr.h"
#include "tr/structures/schema.h"
#include "tr/structures/indirection.h"
#include "tr/vmm/vmm.h"
#include "tr/pstr/pstr.h"
#include "tr/idx/btree/btree.h"
#include "tr/crmutils/node_utils.h"
#include "tr/structures/metadata.h"
#include "tr/idx/index_data.h"
#include "tr/executor/base/PPBase.h"
#include "tr/tr_globals.h"
#include "tr/idx/btree/btintern.h"

#include <map>

// begin of hunk from PPTest.cpp
#define DESC_CONSIST
#define PSTR_CONSIST

static 
xptr get_root (xptr node)
{
	CHECKP(node);
	xptr tmp=node;
	while (true)
	{
      if (((n_dsc*)XADDR(tmp))->pdsc==XNULL) return tmp;
	  tmp=removeIndirection(((n_dsc*)XADDR(tmp))->pdsc);
	}	
}

static
bool is_same_root(xptr x, xptr y)
{
	return get_root(x)==get_root(y);
}

static
void checkTreeConsistency(xptr node)
{
	CHECKP(node);

	check_indirection_consistency(node, false);

	n_dsc* node_d=(n_dsc*)XADDR(node);
	t_nid nd=node_d->nid;
	schema_node* scn=(GETBLOCKBYNODE(node))->snode;
#ifdef DESC_CONSIST
	//1. indirection test
	xptr indir=node_d->indir;
	if (removeIndirection(indir)!=node)
		throw XQUERY_EXCEPTION(SE2030);
	//2. parent test
	CHECKP(node);
	xptr par_indir=node_d->pdsc;
	xptr parent;
	n_dsc* prev_dsc=getPreviousDescriptorOfSameSort(node_d);
	xptr prev_x=(prev_dsc==NULL)?XNULL:ADDR2XPTR(prev_dsc);
    if (par_indir!=XNULL) 
	{
		parent=removeIndirection(par_indir);
		if (!nid_ancestor(parent,node))
			throw XQUERY_EXCEPTION(SE2025);
		if (prev_dsc==NULL|| prev_dsc->pdsc!=par_indir)
		{
			CHECKP(parent);
			xptr* ptr=elementContainsChild((n_dsc*)XADDR(parent),scn->name,scn->type,scn->xmlns);
			if (ptr==NULL || *ptr!=node)
				throw XQUERY_EXCEPTION(SE2026); 
		}
	}
	//3. left siblings + nid comparison
	CHECKP(node);
	xptr left=node_d->ldsc;
	if (left!=XNULL)
	{
		CHECKP(left);
		if (((n_dsc*)XADDR(left))->rdsc!=node)
			throw XQUERY_EXCEPTION(SE2027);
		if (nid_cmp(left,node)>=0)
			throw XQUERY_EXCEPTION(SE2028);
	}
	//4. descriptor's order
	if (prev_x!=NULL && scn->type!=document)
	{
		bool lt=nid_cmp(prev_x,node)<0;
		CHECKP(prev_x);
		if (!lt || getNextDescriptorOfSameSort(prev_dsc)!=node_d   )
		{
			if (is_same_root(prev_x,node))
				throw XQUERY_EXCEPTION(SE2029);
		}
	}
#endif
#ifdef PSTR_CONSIST
	//5.1 nid pstr consistency
	CHECKP(node);
	if (nd.size==0&& is_last_shft_in_blk(*((xptr*)nd.prefix)))
			check_blk_consistency(*((xptr*)nd.prefix));
	//5.2 nid pstr consistency
	CHECKP(node);
	if (scn->textcnt&& ((t_dsc*)node_d)->data!=XNULL&&((t_dsc*)node_d)->size<=PSTRMAXSIZE && is_last_shft_in_blk(((t_dsc*)node_d)->data))
	{
		CHECKP(node);
		check_blk_consistency(((t_dsc*)node_d)->data);	
	}
#endif	
	//recursive walkthrough
	CHECKP(node);
	xptr child=giveFirstByOrderChild(node,CHILDCOUNT(node));
	while (child!=XNULL)
	{
		checkTreeConsistency(child);
		CHECKP(child);
		child=((n_dsc*)XADDR(child))->rdsc;
	}
}
// end of hunk from PPTest.cpp

static FILE *logfile = NULL;
static bool isRcvOK = true;

void test_document(char *name, xptr doc_dsc, bool is_throw)
{
	try
	{
	   checkTreeConsistency(doc_dsc);
       if (!is_throw) fprintf(logfile, "Checked document: %s\n", name);
	}
	catch(SednaException &e) 
	{
       if (is_throw) throw;
       elog(EL_ERROR, ("Recovery failed on document: %s, error: %s\n", name, e.getMsg().c_str()));
       fprintf(logfile, "Recovery failed on document: %s, error: %s\n", name, e.getMsg().c_str());
       isRcvOK = false;
    }
}

void test_indexes(schema_ind_cell* sc_idx)
{
	schema_ind_cell* p = sc_idx;
	
	while (p != NULL)
	{
		try
		{
			bt_check_btree(p->index->btree_root);
       			fprintf(logfile, "Checked index: %s\n", p->index->index_title);
	    	}	
		catch (SednaException &e)
		{
    			elog(EL_ERROR, ("Recovery failed on index: %s, error: %s\n", p->index->index_title, e.getMsg().c_str()));
       			fprintf(logfile, "Recovery failed on index: %s, error: %s\n", p->index->index_title, e.getMsg().c_str());
	    		isRcvOK = false;
    		}

		p = p->next;				
    	}	
}

void test_collection(char *name, col_schema_node *coll)
{
	bt_key key;
	key.setnew(" ");
	bt_cursor cursor = bt_find_gt(coll->metadata->btree_root, key);
			
	if (cursor.is_null()) return;

	do 
	{
		key = cursor.get_key();
		try
		{
			test_document((char*)key.data(), cursor.bt_next_obj(), true);
       		fprintf(logfile, "Checked collection: %s, document: %s\n", name, (char*)key.data());
		}
		catch(SednaException &e) 
		{
       		elog(EL_ERROR, ("Recovery failed on collection: %s, document: %s, error: %s\n", name, (char*)key.data(), e.getMsg().c_str()));
       		fprintf(logfile, "Recovery failed on collection: %s, document: %s, error: %s\n", name, (char*)key.data(), e.getMsg().c_str());
	       	isRcvOK = false;
    	}
	} 
	while(cursor.bt_next_key());

	test_indexes(coll->sc_idx);
}

void test_db_after_rcv()
{
	std::string rcv_fname = std::string(SEDNA_DATA) + std::string("/data/") + std::string(db_name) + std::string("_files/rcv_test_result.log");
	UFile r_fh;

	logfile = fopen(rcv_fname.c_str(), "at");

   	fprintf(logfile, "---------------------------------------------------------------------\n");
	
	metadata_sem_down();

	pers_sset<sn_metadata_cell,unsigned short>::pers_sset_entry* mdc=metadata->rb_minimum(metadata->root);

	while (mdc!=NULL)
    {
    	if (mdc->obj->document_name == NULL)
    		test_collection(mdc->obj->collection_name, (col_schema_node *)mdc->obj->snode);
    	else
    	{
		    xptr blk = ((doc_schema_node *)mdc->obj->snode)->bblk;
		    CHECKP(blk);
			xptr doc_dsc = GETBLOCKFIRSTDESCRIPTORABSOLUTE((node_blk_hdr*)XADDR(blk));
    		test_document(mdc->obj->document_name, doc_dsc, false);
    		test_indexes(((doc_schema_node *)mdc->obj->snode)->sc_idx);
    	}
    
   		mdc=metadata->rb_successor(mdc);
    }

   	metadata_sem_up();

   	fclose(logfile);

#ifdef RCV_TEST_CRASH
   	rcv_fname = std::string(SEDNA_DATA) + std::string("/data/") + std::string(db_name) + std::string("_files");

   	if (isRcvOK)
   		rcv_fname += std::string("/rcv_ok");
   	else
   		rcv_fname += std::string("/rcv_failed");

    r_fh = uCreateFile(rcv_fname.c_str(), U_SHARE_READ | U_SHARE_WRITE, U_READ_WRITE, U_NO_BUFFERING, NULL, NULL);
    if (r_fh == U_INVALID_FD)
  		fprintf(stderr, "Cannot create rcv result file\n");
    uCloseFile(r_fh, NULL);
#endif
}