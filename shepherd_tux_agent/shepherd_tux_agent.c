// shepherd_tux_agent.c was generated by BuildTuxedo on 12/01/16 @ 22:10:45  ***** DO NOT EDIT *****

#include <tmenv.h>
#include <xa.h>
#include <atmi.h>
#include <fml32.h>
#include <tpadm.h>
#include "include_me.h"
static FBFR32 *fml32_machine = NULL;
static FBFR32 *fml32_msg = NULL;
static FBFR32 *fml32_domain = NULL;
static FBFR32 *fml32_server = NULL;

static long max_queues;
static long max_servers;

static int aggregate_stats = 0;
static int show_system = 0;
enum server_sort { SORT_SERVICE, SORT_REQ, SORT_TRX };
static int server_sort_order = SORT_SERVICE;

static FLDID32 fld_msg[] = {
    TA_MSGID,
    TA_MSG_LRPID,
    TA_MSG_LSPID, 
    TA_MSG_QNUM,
    TA_MSG_CBYTES,
    TA_MSG_QBYTES,
    BADFLDID
};

static FLDID32 fld_server[] = {
    TA_SERVERNAME,
    TA_SRVID,
    TA_PID,
    TA_RPID,
    TA_RQID,
    TA_NUMREQ,
    TA_NUMTRAN,
    TA_NUMTRANABT,
    TA_NUMTRANCMT,
    TA_CURRSERVICE,
    TA_TOTREQC,
    BADFLDID
};

enum qtypes { UNKNOWN = 0, IN, INMANY, INOUT, OUT };
static const char *strqtype(int qtype)
{
    if (qtype == IN) {
        return "  -> ";
    } else if (qtype == INMANY ) {
        return "  => ";
    } else if (qtype == INOUT) {
        return " <-> ";
    } else if (qtype == OUT ) {
        return "  -# ";
    } else {
        return " ??? ";
    }
}

struct tux_server;
struct tux_queue {
    long msgid;

    long qnum;
    long cbytes;
    long qbytes;

    double used;

    long lrpid;
    long lspid;

    int qtype;
    struct tux_server *server;
    struct tux_server *sender;
    long sender_pid;
};
struct tux_domain{
	long ipckey;
	char master[32];
	char model[10];
	char state[10];
	char domain_id[32];
	long perm;
	long licexpire;
	long licmaxusers;
	char licserial[80];
	long maxaccessers;
	long maxconv;
	long maxgtt;
	long maxgroups;
	long maxmachines;
	long maxqueues;
	long maxservers;
	long maxservices;
	char ldbal[5];
	char system_access[10];
	long scanunit;
	long bblquery;
	long blocktime;
	long sanityscan;
	long curdrt;
	long curgroups;
	long curmachines;
	long curservers;
	long curservices;
	long hwgroups;
	long hwmachines;
	long hwservers;
	long hwservices;

};
struct tux_machine {
    char lmid[30];
    char pmid[30];
	char tuxconfig[50];
	char appdir[50];
	char ulogpfx[50];

	long maxclients;

    long n_tran;
    long n_tranabt;
    long n_trancmt;

    long n_req;
    long n_enqueue;
    long n_dequeue;

    long n_accessers;
    long n_clients;
    long n_gtt;

    long n_queues;
    long n_servers;
    long n_services;
};

struct tux_server {
	char srvgrp[40];
	long srvid;
	char servername[85];
	long grpno;
	char state[10];
	long basesrvid;
	char clopt[1024];
	char envfile[256];
	long grace;
	long maxgen;
	long max;
	long min£»
	long mindispatchthreads£»
	long maxdispatchthreads£»
	long threadstacksize£»
	long curdispatchthreads£»
	long hwdispatchthreads£»
	long numdispatchthreads£»
	char rcmd[256];
	char restart[5];
	long sequence;
	char system_access[20];
	char conv[5];
	char replyq[5];
	long rpperm;
	char rqaddr[30];
	long rqperm;
	char lmid[50];
	long generation;
	long pid;
	long rpid;
	long rqid;
	long timerestart;
	long timestart;
};

struct tux_server_stat {
    double n_req;
    double n_tran;
    double n_tranabt;
    double n_trancmt;
    double reqc;
    long n;
    struct tux_server *server;
};


static struct tux_server_stat *server_stats = NULL;
static struct tux_server_stat *tmp_server_stats = NULL;
static int n_server_stats = 0;
struct tux_machine machine;
struct tux_domain domain;
struct tux_queue *queues;
struct tux_server servers;


int tpcall_fml32(const char *svc, FBFR32 *idata, FBFR32 **odata, long flags)
{
    long olen = 0;

	if(tpcall((char *)svc, (char *)idata, 0, (char **)odata, &olen, flags) != -1){
            printf("Failed to call %s, %s\n", svc, tpstrerror(tperrno));
			return -1;
	}
    return 0;

}

int Fchg32_string(FBFR32 **fbfr, FLDID32 fieldid, FLDOCC32 oc, const char *value)
{
    for (;;) {
        if (Fchg32(*fbfr, fieldid, oc, (char *)value, 0) != -1) {
            break;
        }

		if(Ferror32 != FNOSPACE){
			printf("fml32 error: %s\n", Fstrerror32(Ferror32));
			return -1;
		}
		if((*fbfr = (FBFR32 *)tprealloc((char *)*fbfr, Fsizeof32(*fbfr) * 2)) == NULL){
			printf("tprealloc failed\n");
			return -1;
		}
    }
    return 0;

}

int Fchg32_long(FBFR32 **fbfr, FLDID32 fieldid, FLDOCC32 oc, long value)
{
    for (;;) {
        if (Fchg32(*fbfr, fieldid, oc, (char *)&value, 0) != -1) {
            break;
        }
		
		if(Ferror32 != FNOSPACE){
			printf("fml32 error: %s", Fstrerror32(Ferror32));
			return -1;
		}
		if((*fbfr = (FBFR32 *)tprealloc((char *)*fbfr, Fsizeof32(*fbfr) * 2)) == NULL){
			return -1;
			printf("tprealloc failed\n");
		}
    }
    return 0;
}

/* NOTE: this one does string truncation unlike the original Fget32 that fails with FNOSPACE */
int Fget32_string(FBFR32 *fbfr, FLDID32 fieldid, FLDOCC32 oc, char *loc, FLDLEN32 maxlen)
{
    char *val;
	if((val = Ffind32(fbfr, fieldid, oc, NULL)) == NULL){
		printf("fml32 error on field %ld: %s\n", (long)fieldid, Fstrerror32(Ferror32));
			return -1;
	}
    
    strncpy(loc, val, maxlen);
    loc[maxlen - 1] = '\0';
    return 0;
}

int Fget32_long(FBFR32 *fbfr, FLDID32 fieldid, FLDOCC32 oc, long *loc)
{
	if(Fget32(fbfr, fieldid, oc, (char *)loc, NULL) == -1){
		printf("fml32 error on field %ld: %s\n", (long)fieldid, Fstrerror32(Ferror32));
			return -1;
	}
    return 0;
}




int join_application(const char *name)
{
    TPINIT *tpinitbuf = NULL;

    tpinitbuf = (TPINIT *)tpalloc("TPINIT", NULL, TPINITNEED(16));
	if(tpinitbuf == NULL){
		printf("Failed to allocate TPINIT\n");
		return -1;
	}
	 strcpy(tpinitbuf->usrname, name) ;
     strcpy(tpinitbuf->passwd, "") ;
     strcpy(tpinitbuf->cltname, "tpsysadm") ;

	if(tpinit(tpinitbuf) == -1){
		printf("Failed to join application, %s\n", tpstrerror(tperrno));
		return -1;
	}else{
		printf("Server connect success\n");
	}
    tpfree((char *)tpinitbuf) ;

    return 0;

}

int tmibcall(const char *what,FBFR32 **resp){
	long blen;
	FBFR32 *fml32_req = NULL;
	if (fml32_req == NULL) {
		fml32_req = (FBFR32 *)tpalloc(FMLTYPE32,NULL,0);
		if(fml32_req == NULL){
			printf("Failed to allcate send buffer\n");
		}
	}
	Finit32(fml32_req, Fsizeof32(fml32_req));
	if(Fchg32(fml32_req,TA_OPERATION,0,"GET",0) == -1){
			printf("Failed to set TA_OPERATION %s\n",tpstrerror(tperrno));
	}
	if(Fchg32(fml32_req, TA_CLASS, 0, what,0) == -1 ){
		printf("Failed to set TA_CLASS %s\n",tpstrerror(tperrno));
		return -1;
	}
	 if (tpcall(".TMIB",fml32_req,0,resp,&blen,0)== -1){
              printf("tpcall(.TMIB) failed:%s\n",tpstrerror(tperrno));;
              return -1;
     }
	return 0;

}

init_state(){
	tuxputenv("WSNADDR=//120.26.79.234:5555");
    join_application("shepherd");
	//allocate recv fml buffer
	if((fml32_machine = (FBFR32 *)tpalloc("FML32", NULL, 4 * 1024)) == NULL){
		printf("fml32_machine allocate failed\n");
	}
	if((fml32_domain = (FBFR32 *)tpalloc("FML32", NULL, 4 * 1024)) == NULL){
		printf("fml32_domain allocate failed\n");
	}
	if((fml32_msg = (FBFR32 *)tpalloc("FML32", NULL, 4 * 1024)) == NULL){
		printf("fml32_msg allocate failed\n");
	}
	if((fml32_server = (FBFR32 *)tpalloc("FML32", NULL, 4 * 1024)) == NULL){
		printf("fml32_server allocate failed\n");
	}

	//init metrics
	if(tmibcall("T_DOMAIN",&fml32_domain) == -1){
		printf("Failed to call T_MACHINE\n");
	}
	if(tmibcall("T_MACHINE",&fml32_machine) == -1){
		printf("Failed to call T_MACHINE\n");
	}
	if(tmibcall("T_MSG",&fml32_msg) == -1){
		printf("Failed to call T_MSG\n");
	}
	if(tmibcall("T_SERVER",&fml32_server) == -1){
		printf("Failed to call T_SERVER\n");
	}
}
parse_tx_domain(){
	Fget32_string(fml32_machine, TA_LMID, 0, machine.lmid, sizeof(machine.lmid));
	
	Fget32_string(fml32_domain, TA_MASTER, 0, domain.master, sizeof(domain.master));
	Fget32_string(fml32_domain, TA_MODEL, 0, domain.model, sizeof(domain.model));
	Fget32_string(fml32_domain, TA_STATE, 0, domain.state, sizeof(domain.state));
	Fget32_string(fml32_domain, TA_DOMAINID, 0, domain.domain_id, sizeof(domain.domain_id));
	Fget32_string(fml32_domain, TA_LICSERIAL, 0, domain.licserial, sizeof(domain.licserial));
	Fget32_string(fml32_domain, TA_LDBAL, 0, domain.ldbal, sizeof(domain.ldbal));
	Fget32_string(fml32_domain, TA_SYSTEM_ACCESS, 0, domain.system_access, sizeof(domain.system_access));
	Fget32_long(fml32_domain, TA_IPCKEY, 0, &domain.ipckey);
	Fget32_long(fml32_domain, TA_PERM, 0, &domain.perm);
	Fget32_long(fml32_domain, TA_LICEXPIRE, 0, &domain.licexpire);
	Fget32_long(fml32_domain, TA_LICMAXUSERS, 0, &domain.licmaxusers);
	Fget32_long(fml32_domain, TA_SCANUNIT, 0, &domain.scanunit);
	Fget32_long(fml32_domain, TA_BBLQUERY, 0, &domain.bblquery);
	Fget32_long(fml32_domain, TA_BLOCKTIME, 0, &domain.blocktime);
	Fget32_long(fml32_domain, TA_DBBLWAIT, 0, &domain.sanityscan);
	Fget32_long(fml32_domain, TA_CURDRT, 0, &domain.curdrt);
	Fget32_long(fml32_domain, TA_CURGROUPS, 0, &domain.curgroups);
	Fget32_long(fml32_domain, TA_CURMACHINES, 0, &domain.curmachines);
	Fget32_long(fml32_domain, TA_CURSERVERS, 0, &domain.curservers);
	Fget32_long(fml32_domain, TA_CURSERVICES, 0, &domain.curservices);
	Fget32_long(fml32_domain, TA_HWSERVERS, 0, &domain.hwservers);
	Fget32_long(fml32_domain, TA_HWSERVICES, 0, &domain.hwservices);
	Fget32_long(fml32_domain, TA_MAXACCESSERS, 0, &domain.maxaccessers);
	Fget32_long(fml32_domain, TA_MAXCONV, 0, &domain.maxconv);
	Fget32_long(fml32_domain, TA_MAXGTT, 0, &domain.maxgtt);
	Fget32_long(fml32_domain, TA_MAXGROUPS, 0, &domain.maxgroups);
	Fget32_long(fml32_domain, TA_MAXSERVICES, 0, &domain.maxservices);
	Fget32_long(fml32_domain, TA_MAXQUEUES, 0, &domain.maxqueues);
	Fget32_long(fml32_domain, TA_MAXSERVERS, 0, &domain.maxservers);
	
	printf("ipckey:%ld\n",domain.ipckey);
	printf("master:%s\n",domain.master);
	printf("model:%s\n",domain.model);
	printf("state:%s\n",domain.state);
	printf("domain_id:%s\n",domain.domain_id);
	printf("perm:%ld\n",domain.perm);
	printf("licexpire:%ld\n",domain.licexpire);
	printf("licmaxusers:%ld\n",domain.licmaxusers);
	printf("licserial:%s\n",domain.licserial);
	printf("maxaccessers:%ld\n",domain.maxaccessers);
	printf("maxconv:%ld\n",domain.maxconv);
	printf("maxgtt:%ld\n",domain.maxgtt);
	printf("maxgroups:%ld\n",domain.maxgroups);
	printf("maxmachines:%ld\n",domain.maxmachines);
	printf("maxqueues:%ld\n",domain.maxqueues);
	printf("maxservers:%ld\n",domain.maxservers);
	printf("maxservices:%ld\n",domain.maxservices);
	printf("ldbal:%s\n",domain.ldbal);
	printf("system_access:%s\n",domain.system_access);
	printf("scanunit:%ld\n",domain.scanunit);
	printf("bblquery:%ld\n",domain.bblquery);
	printf("blocktime:%ld\n",domain.blocktime);
	printf("sanityscan:%ld\n",domain.sanityscan);
	printf("curdrt:%ld\n",domain.curdrt);
	printf("curgroups:%ld\n",domain.curgroups);
	printf("curmachines:%ld\n",domain.curmachines);
	printf("curservers:%ld\n",domain.curservers);
	printf("curservices:%ld\n",domain.curservices);
	printf("hwgroups:%ld\n",domain.hwgroups);
	printf("hwmachines:%ld\n",domain.hwmachines);
	printf("hwservers:%ld\n",domain.hwservers);
	printf("hwservices:%ld\n",domain.hwservices);
}
parse_tx_machine(){
	Fget32_string(fml32_machine, TA_LMID, 0, machine.lmid, sizeof(machine.lmid));
	Fget32_string(fml32_machine, TA_PMID, 0, machine.pmid, sizeof(machine.pmid));
	Fget32_string(fml32_machine, TA_TUXCONFIG, 0, machine.tuxconfig, sizeof(machine.tuxconfig));
	Fget32_string(fml32_machine, TA_APPDIR, 0, machine.appdir, sizeof(machine.appdir));
	Fget32_string(fml32_machine, TA_ULOGPFX, 0, machine.ulogpfx, sizeof(machine.ulogpfx));
	Fget32_long(fml32_machine, TA_MAXACCESSERS, 0, &machine.maxclients);
	printf("lid:%s\tpmid:%s\t tuxconfig:%s\tappdir:%s\tulogpfx:%s\tmaxwxclients:%ld\t\n",machine.lmid,machine.pmid,machine.tuxconfig,machine.appdir,machine.ulogpfx,machine.maxclients);

}
parse_tx_group(){

}
parse_tx_server(){
	Fget32_string(fml32_server, TA_SERVERNAME, 0, servers.name,sizeof(servers.name));
	printf("server Name:%s\n",servers.name);
}
parse_tx_service(){

}
parse_dm_local(){

}
parse_dm_remote(){

}
parse_dm_network(){

}
parse_dm_imported(){

}
parse_dm_exported(){
}

cleanup(){
	 tpfree((char *)fml32_machine);
	   tpfree((char *)fml32_domain);
	   tpfree((char *)fml32_msg);
	   tpfree((char *)fml32_server);
       tpterm();

}




int main(int argc,char *argv[]){
	init_state();
	parse_tx_domain();
	parse_tx_machine();
	/*
	parse_tx_machine();
	parse_tx_server();

	Fget32_long(fml32_machine, TA_NUMENQUEUE, 0, &machine.n_enqueue);

	Fget32_long(fml32_machine, TA_NUMDEQUEUE, 0, &machine.n_dequeue);

	Fget32_long(fml32_machine, TA_NUMTRAN, 0, &machine.n_tran);
	Fget32_long(fml32_machine, TA_NUMTRANABT, 0, &machine.n_tranabt);
	Fget32_long(fml32_machine, TA_NUMTRANCMT, 0, &machine.n_trancmt);
	Fget32_long(fml32_machine, TA_CURACCESSERS, 0, &machine.n_accessers);
	Fget32_long(fml32_machine, TA_CURCLIENTS, 0, &machine.n_clients);
	Fget32_long(fml32_machine, TA_CURGTT, 0, &machine.n_gtt);
	Fget32_long(fml32_domain, TA_CURQUEUES, 0, &machine.n_queues);
	Fget32_long(fml32_domain, TA_CURSERVICES, 0, &machine.n_services);
	Fget32_long(fml32_domain, TA_CURSERVERS, 0, &machine.n_servers);
	*/
	
    printf("\n");

	cleanup();
	return 0;
}