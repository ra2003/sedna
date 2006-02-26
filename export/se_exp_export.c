#include "se_exp_common.h"
#include "se_exp_queries.h"

// function exports data from database to the specified directory
int export(const char * path,const char *url,const char *db_name,const char *login,const char *password) {
  struct SednaConnection conn;
  qbuf_t exp_docs = {NULL,0,0};
  qbuf_t load_docs = {NULL,0,0};
  qbuf_t create_colls = {NULL,0,0};
  qbuf_t create_indexes = {NULL,0,0};
  qbuf_t create_sec = {NULL,0,0};
  int i,res,error_status=1;
  FILE *log,*f;
  char strbuf[PATH_SIZE];


    sprintf(strbuf,"%s%s",path,EXP_LOG_FILE_NAME);

	if ((log=fopen(strbuf,"w"))==NULL) {
		printf("ERROR: path '%s' is not accessible for writing\n",path);
		goto exp_error_no_conn;
	}
	
	FTRACE((log,"Connecting to Sedna..."));
	if(SEconnect(&conn, url, db_name, login, password)!= SEDNA_SESSION_OPEN) {
		ETRACE((log,"ERROR: can't connect to Sedna XML DB\n%s\n", SEgetLastErrorMsg(&conn)))
		goto exp_error_no_conn;
	}
	FTRACE((log,"done\n"));
	
    FTRACE((log,"Starting transaction..."));
	if ((res = SEbegin(&conn))!= SEDNA_BEGIN_TRANSACTION_SUCCEEDED) {
		ETRACE((log,"ERROR: failed to begin transaction\n"))
		goto exp_error;
	}
	FTRACE((log,"done\n"));
			
	FTRACE((log,"Constructing export documents script"));
	if (fill_qbuf(&conn,&exp_docs, exp_docs_query, log)!=0) {
		goto exp_error;
	}
	FTRACE((log,"...done (%d statements)\n", exp_docs.d_size));

	FTRACE((log,"Constructing load documents script"));
	if (fill_qbuf(&conn,&load_docs, load_docs_query, log)!=0) {
		goto exp_error;
	}
	FTRACE((log,"...done (%d statements)\n", load_docs.d_size));

	FTRACE((log,"Constructing create collections script"));
	if (fill_qbuf(&conn,&create_colls, create_colls_query, log)!=0) {
		goto exp_error;
	}
	FTRACE((log,"...done (%d statements)\n", create_colls.d_size));

	FTRACE((log,"Constructing create indexes script"));
	if (fill_qbuf(&conn,&create_indexes, create_indexes_query, log)!=0) {
		goto exp_error;
	}
	FTRACE((log,"...done (%d statements)\n", create_colls.d_size));

	FTRACE((log,"Constructing export security script"));
	if (fill_qbuf(&conn,&create_sec, create_sec_query, log)!=0) {
		goto exp_error;
	}
	FTRACE((log,"...done (%d statements)\n", create_sec.d_size));


	for (i=0;i<exp_docs.d_size;i++) {
		/* workaround through sedna API error */
		char *sedna_bug_nl = exp_docs.buf[i];
		while (*sedna_bug_nl=='\n') sedna_bug_nl++;
		/* end */
		FTRACE((log,"Exporting document %d of %d [%s]...",(i+1),exp_docs.d_size,sedna_bug_nl));
		sprintf(strbuf,"%s%d.xml",path,(i+1));
		if ((f=fopen(strbuf,"w"))==NULL) {
			ETRACE((log,"ERROR: can't write to file %s\n",strbuf))
			goto exp_error;
		}
		if (execute_query(&conn,exp_docs.buf[i],f,log)!=0) 
			goto exp_error;
		fclose(f);
		FTRACE((log,"done\n"));
	}

	FTRACE((log,"Exporting security data..."));
	sprintf(strbuf,"%s%s.xml",path,DB_SECURITY_DOC);
	if ((f=fopen(strbuf,"w"))==NULL) {
		ETRACE((log,"ERROR: can't write to file %s\n",strbuf))
		goto exp_error;
	}
	sprintf(strbuf,"doc('%s')",DB_SECURITY_DOC);
	if (execute_query(&conn,strbuf,f,log)!=0) 
		goto exp_error;
	fclose(f);
	FTRACE((log,"done\n"));

	FTRACE((log,"Writing XQuery scripts..."))
	sprintf(strbuf,"%s%s",path,CR_COL_QUERY_FILE);
	write_xquery_script(&create_colls,strbuf);

	sprintf(strbuf,"%s%s",path,LOAD_DOCS_QUERY_FILE);
	write_xquery_script(&load_docs,strbuf);

	sprintf(strbuf,"%s%s",path,CR_INDEXES_QUERY_FILE);
	write_xquery_script(&create_indexes,strbuf);

	sprintf(strbuf,"%s%s",path,CR_SEC_QUERY_FILE);
	write_xquery_script(&create_sec,strbuf);
	FTRACE((log,"done\n"));

	FTRACE((log,"Commiting the transaction..."))
	SEcommit(&conn);
	FTRACE((log,"done\n"));

	error_status=0;

exp_error:

	FTRACE((log,"Closing connection..."))
	SEclose(&conn);
	FTRACE((log,"done\n"))
	

//disposing dynamic memory
exp_error_no_conn:
	if (log!=NULL) fclose(log);
	for (i=0;i<exp_docs.d_size;i++) if (exp_docs.buf[i]!=NULL) {free(exp_docs.buf[i]); exp_docs.buf[i]=NULL;}
	for (i=0;i<load_docs.d_size;i++) if (load_docs.buf[i]!=NULL) {free(load_docs.buf[i]); load_docs.buf[i]=NULL;}
	for (i=0;i<create_colls.d_size;i++) if (create_colls.buf[i]!=NULL) {free(create_colls.buf[i]); create_colls.buf[i]=NULL;}
	for (i=0;i<create_sec.d_size;i++) if (create_sec.buf[i]!=NULL) {free(create_sec.buf[i]); create_sec.buf[i]=NULL;}
	if (exp_docs.buf!=NULL) free(exp_docs.buf);
	if (load_docs.buf!=NULL) free(load_docs.buf);
	if (create_colls.buf!=NULL) free(create_colls.buf);
	if (create_sec.buf!=NULL) free(create_sec.buf);
	if (error_status==1)
		return -1;
	else
		return 0;
}