#ifndef _FT_SEARCH_H
#define _FT_SEARCH_H

#include "dm_accessors.h"
#include "strings.h"
#include "e_string.h"
#include "e_string_iterator.h"
#include "ft_index_data.h"

#include <ios>
#include <sstream>
#include <windows.h>
#define USE_DTSEARCH_NAMESPACE
#include <dtsfc.h>
#include "crmutils.h" 


class SednaTextInputStream {
    public:
        SednaTextInputStream(dtsFileInfo* _info_,ft_index_type _cm_,pers_sset<ft_custom_cell,unsigned short>* _custom_tree_);
        ~SednaTextInputStream();
        void seek_mem(long where);
        int read_mem(void *dest, long bytes);
        void seek_estr(long where);
        int read_estr(void *dest, long bytes);
        void makeInterface(dtsInputStream& dest,xptr& node);
        static void seekCBmem(void *pData, long where);
        static long readCBmem(void *pData, void *data, long bytes);
        static void seekCBestr(void *pData, long where);
        static long readCBestr(void *pData, void *data, long bytes);
        static void releaseCB(void *pData);
        static SednaTextInputStream *safeCast(void *pData);
    protected:
        long idTextInputStream;
        long pos;
		e_string_iterator estr_it;
        dtsFileInfo* fileInfo;
		t_str_buf in_buf;
		ft_index_type cm;
		pers_sset<ft_custom_cell,unsigned short>* custom_tree;
    };

class SednaDataSource : public dtSearch::DDataSourceBase {
    public:
        SednaDataSource(ft_index_type _cm_,pers_sset<ft_custom_cell,unsigned short>* _custom_tree_);
		~SednaDataSource(){delete tis;}
        virtual int getNextDoc(dtsInputStream& s);
		virtual xptr get_next_doc()=0;
		virtual int rewind();
        static void recordToFilename(char *dest,xptr _node_);
        static xptr filenameToRecord(const char *dest);
//		dtsDataSource * getInterface();
    protected:
		ft_index_type cm;
		pers_sset<ft_custom_cell,unsigned short>* custom_tree;
		SednaTextInputStream *tis;
		dtsFileInfo fileInfo;

    };
class OperationSednaDataSource : public  SednaDataSource
{
public:
	OperationSednaDataSource(ft_index_type _cm_,pers_sset<ft_custom_cell,unsigned short>* _custom_tree_,PPOpIn* _op_);
private:
	virtual xptr get_next_doc();
	virtual int rewind();
	PPOpIn* op;
	tuple t;
};
class CreationSednaDataSource : public  SednaDataSource
{
public:
	CreationSednaDataSource(ft_index_type _cm_,pers_sset<ft_custom_cell,unsigned short>* _custom_tree_,std::vector<xptr>* _first_nodes_);
private:
	virtual xptr get_next_doc();
	virtual int rewind();
	std::vector<xptr>* first_nodes;
	std::vector<xptr>::iterator it;
	xptr tmp;
};
class UpdateSednaDataSource : public  SednaDataSource
{
public:
	UpdateSednaDataSource(ft_index_type _cm_,pers_sset<ft_custom_cell,unsigned short>* _custom_tree_,xptr_sequence * _seq_);
private:
	virtual xptr get_next_doc();
	virtual int rewind();
	xptr_sequence * seq;
	xptr_sequence::iterator it;
};
class SednaConvertJob
{
public:
	SednaConvertJob(ft_index_type _cm_,pers_sset<ft_custom_cell,unsigned short>* _custom_tree_);
	void convert_node(xptr& node,long* ht,long ht_cnt);
	virtual void OnOutput(const char * txt, int length);
	static e_str_buf result;
private:
	static t_str_buf in_buf;
	SednaTextInputStream *tis;
	dtsFileInfo fileInfo;

	int current_word;
	int current_ht_idx;
	int ht_cnt;
	long *ht;
	ft_index_type cm;
	pers_sset<ft_custom_cell,unsigned short>* custom_tree;
	template <class Iterator> void parse_doc(Iterator &str_it, Iterator &str_end);
	template <class Iterator> int getch(Iterator &str_it, Iterator &str_end);
	void putch(const int ch);
	int cur_ch;
	static const int EOF_ch = -1;
	template <class Iterator> void parse_tag(Iterator &str_it, Iterator &str_end);
};
class SednaSearchJob : public dtSearch::DSearchJob {
     public:
           
           virtual void OnError(long errorCode, const char *msg);
           virtual void OnFound(long totalFiles,
                 long totalHits, const char *name, long hitsInFile, dtsSearchResultsItem& item);
		   virtual void OnSearchingIndex(const char * indexPath);
		   SednaSearchJob(PPOpIn* _seq_,ft_index_type _cm_,pers_sset<ft_custom_cell,unsigned short>* _custom_tree_,bool _hilight_=false);		   
		   SednaSearchJob(bool _hilight_=false);		   
		   void set_request(tuple_cell& request);
		   void get_next_result(tuple &t);
		   void set_index(tuple_cell& name);
		   void reopen();        
		   virtual ~SednaSearchJob();
		   static DWORD WINAPI ThreadFunc( void* lpParam );
	  private:
		  PPOpIn* seq;
		  UTHANDLE dtth;
		  UUnnamedSemaphore sem1,sem2;
		  long save_field_flags;
		  xptr res;
		  bool hilight;
		  SednaConvertJob * hl;

     };


#endif