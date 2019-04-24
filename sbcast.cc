#include "sbcast.h"
#include <random.h>
#include <time.h>
#include "sbcast_output.h"

#define CODE_NUM 3

#define BUF 1000

#define S 0
#define R 1
#define F 2
#define C 3

#define PKT_NORMAL 0
#define PKT_CODED 1
#define PKT_REQUEST 3
#define PKT_REPLY 4

#define STA_FL 0
#define STA_CODEWAIT 1
#define STA_CODESENDREADY 2
#define STA_HYBRID 3


#define SWITCH_TH 0.3  //float
#define TIME_TH 5.0    //float
#define NEIGHBOR_TH 2  //int

#define NODE_NUM 18
#define GALOIS 256
#define DELAY 1.0
#define MAX_PACKET 750

#define TRANSTH_TYPE 1
#define UNDER_TH 0.0
#define UPPER_TH 0.0

FILE * mytraceFile=fopen ("mytrace.tr","wt");
FILE * codelogFile=fopen ("codelog.tr","wt");
FILE * codelogFile2=fopen ("codelog2.tr","wt");
FILE * historyFile=fopen ("hist.tr","wt");
FILE * recvlogFile=fopen ("recvlog.tr","wt");
FILE * createcodeFile=fopen ("createcode.tr","wt");
FILE * recvhistoryFile=fopen ("rcvhist.tr","wt");
FILE * recvcodehistoryFile=fopen ("rcvcodehist.tr","wt");
FILE * resFile=fopen ("res.tr","wt");
FILE * tpFile=fopen ("tp.tr","wt");
FILE * tempFile=fopen ("temp.tr","wt");
FILE * temp2File=fopen ("temp2.tr","wt");



// TCL Hooks
int hdr_beacon::offset_;

static class SBcastHeaderClass : public PacketHeaderClass {
public:
	SBcastHeaderClass():PacketHeaderClass("PacketHeader/SB",sizeof(hdr_beacon)) {
		bind_offset(&hdr_beacon::offset_);
	}
}class_hdr_sbcast;

static class SBcastClass : public TclClass {
public:
	SBcastClass():TclClass("Agent/SB") { }
	TclObject *create(int argc,const char*const* argv) {
		return (new SBAgent());
	}
}class_sbagent;

SBAgent::SBAgent():Agent(PT_SB),btimer_(this) { }

int SBAgent::command(int argc,const char*const* argv) {
	if(argc==2){
		if(strcmp(argv[1],"start")==0){
			my_addr_ = addr();
			return TCL_OK;
		}
		if(strcmp(argv[1],"base-station")==0){
			btimer_.resched((double)1.0);//1パケット目の遅延
			my_addr_ = addr();
			return TCL_OK;
		}

	}else if (argc == 3) {
		// Obtains corresponding dmux to carry packets to upper layers
		if (strcmp(argv[1], "port-dmux") == 0) {
			dmux_ = (PortClassifier*)TclObject::lookup(argv[2]);
			if (dmux_ == 0) {
				fprintf(stderr, "%s: %s lookup of %s failed\n",
						__FILE__,
						argv[1],
						argv[2]);
				return TCL_ERROR;
			}
			return TCL_OK;
		}

		if (strcmp(argv[1], "log-target") == 0 ||
			strcmp(argv[1], "tracetarget") == 0) {
			logtarget_ = (Trace*)TclObject::lookup(argv[2]);
			if (logtarget_ == 0)
				return TCL_ERROR;
			return TCL_OK;
		}
	}
	return (Agent::command(argc,argv));
}

void BcastTimer::expire(Event *e) {
	agent_->sendBeacon();
	agent_->resetBcastTimer();

}

void SBAgent::resetBcastTimer() {
	// Reschedule the timer for every 0.1 seconds
	btimer_.resched((double)0.25);

}



static int pktnum=1;
static int codepktnum=1;
static int recvlog[BUF][BUF];
static int recvcodecount[BUF];
static int recvcodelog[BUF][BUF];
static int recvcodevec[BUF][BUF];
static int recvcode1[BUF][BUF];
static int recvcode2[BUF][BUF];
static int recvcode3[BUF][BUF];
static int recvcode4[BUF][BUF];
static int recvcode5[BUF][BUF];
static int recvcodeFlag[BUF][BUF];
static int sendcodelog[BUF][BUF];


static int mystatus[BUF];
static int collectlist[BUF][BUF];
static int collectNum[BUF];
static int dcwait1[BUF][BUF];
static int dcwait2[BUF][BUF];
static int dcwait3[BUF][BUF];
static int dcwait4[BUF][BUF];
static int dcwait5[BUF][BUF];
static int dcwaitvec[BUF][BUF];
static int decodequeuecount[BUF];

static int sender[BUF][BUF];
static int packetweight[BUF][BUF];
static float recvtime[BUF][BUF];
static int sendercount[BUF];
static int topocount[BUF];
static int topo[BUF];
static int hitcount[BUF];

static int all_rcv_ncount;
static int all_rcv_ccount;
static int all_send_ncount;
static int all_send_ccount;
//static int ref[];

static int packet_count;

//int SBAgent::createCodepacket2(int,int);
void SBAgent::sendRequest(){

}

void SBAgent::sendBeacon() {
    static int syokika=0;

    if(syokika==0){//初期化
        for(int i=0;i<BUF;i++){
            for(int j=0;j<BUF;j++){
                recvlog[i][j]=0;
				recvcodecount[i]=0;
				recvcodevec[i][j]=0;
				recvcodelog[i][j]=0;
				recvcode1[i][j]=0;
				recvcode2[i][j]=0;
				recvcode3[i][j]=0;
				recvcode4[i][j]=0;
				recvcode5[i][j]=0;
				recvcodeFlag[i][j]=0;
                collectlist[i][j]=-2;
                sender[i][j]=-1;
                packetweight[i][j]=0;
                recvtime[i][j]=-1;
                dcwait1[i][j]=-1;
				dcwait2[i][j]=-1;
				dcwait3[i][j]=-1;
				dcwait4[i][j]=-1;
				dcwait5[i][j]=-1;
				dcwaitvec[i][j]=-1;
                sendcodelog[i][j]=-1;
            }
            mystatus[i]=0;
            collectNum[i]=0;
            sendercount[i]=0;
            topocount[i]=0;
            topo[i]=0;
            hitcount[i]=0;
            decodequeuecount[i]=0;
        }
        syokika=1;
        all_rcv_ccount=0;
        all_rcv_ncount=0;
        all_send_ccount=0;
        all_send_ncount=0;
        packet_count=0;
        int a=test(1, 2);
        fprintf(mytraceFile,"test:%d\n",a);
        time_t t = time(NULL);
        srand((unsigned) time(NULL));
        int seed = rand()%256+1;
        Random::seed(seed);
        fprintf(mytraceFile,"**************************************************\n");
        fprintf(mytraceFile,"%s",ctime(&t));
        fprintf(mytraceFile,"SWITCH_TH:%f ",SWITCH_TH);
		fprintf(mytraceFile,"TIME_TH:%f ",TIME_TH);
		fprintf(mytraceFile,"NEIGHBOR_TH:%d\n",NEIGHBOR_TH);


		fprintf(mytraceFile,"DELAY:%f\n",DELAY);
        fprintf(mytraceFile,"SEED:%d\n",seed);
        fprintf(mytraceFile,"GALOIS:%d\n",GALOIS);
        fprintf(mytraceFile,"**************************************************\n");

        fprintf(mytraceFile,"event\ttime\tnode\tfrom\tPKTtype\tPKTno\tcv\ttopo\tstatus\tnei\n");
    }
	Packet* p = allocpkt();
	struct hdr_cmn* ch = HDR_CMN(p);
	struct hdr_ip* ih = HDR_IP(p);
	struct hdr_beacon * ph = HDR_BEACON(p);

	ph->addr() = my_addr();
	ph->seq_num() = seq_num_++;

	ph->pktnum_=pktnum;
	ph->pkttype_=PKT_NORMAL;
	ch->ptype() = PT_SB;
	ph->pkt1_=-1;
	ph->pkt2_=-1;
	ph->pkt3_=-1;
	ph->pkt4_=-1;
	ph->pkt5_=-1;
	ph->hop_count_=1;
	ph->encode_count_=-1;

	ch->direction() = hdr_cmn::DOWN;
	ch->size() = IP_HDR_LEN;
	ch->error() = 0;
	ch->next_hop() = IP_BROADCAST;
	ch->addr_type() = NS_AF_INET;

	ih->saddr() = my_addr();
	ih->daddr() = IP_BROADCAST;
	ih->sport() = RT_PORT;
	ih->dport() = RT_PORT;
	ih->ttl() = IP_DEF_TTL;
	fprintf(mytraceFile, "s\t%f\tnode:%d\tfrom:*\ttype:N\tpktNo:%d\n", Scheduler::instance().clock(), my_addr(),ph->pktnum_);
    //ch->uid()=-1;
	fprintf(stdout,"uid %d\n",ch->uid());
    long int pktid;
    int id=-1;
    if (id < 0) { //new broadcast pkt
//        pktid=ch->uid();
    }
    else { /* forwarding a broadcast pkt, same id and flow id are setted */
        pktid=id;
    }
    //即時送信
	send(p,0);
    test2('s',my_addr());
    //遅延送信
    //Scheduler::instance().schedule(target_,p,DELAY * Random::uniform());
	pktnum++;
}

void SBAgent::recv(Packet *p,Handler *h) {

    struct hdr_cmn *ch = hdr_cmn::access(p);
    struct hdr_ip *ih = hdr_ip::access(p);
    struct hdr_beacon *ph = hdr_beacon::access(p);

    int flag = 0;



    //全パケット共通動作
    fprintf(stdout, "-----------node:%d rcv Start--------------------------\n", my_addr());
    fprintf(stdout, "r time:%f, type:%d,  num:%d, sender:%d\n", Scheduler::instance().clock(), ph->pkttype_,
            ph->pktnum_, ph->addr());
    //送信ノードと自分自身のパケットは破棄
    if (ph->addr() == my_addr() || my_addr() == 0) {
        return;
    }
    //パケットカウント
    if(Scheduler::instance().clock()>30 ) {
        //packet_count++;
    }
    if(Scheduler::instance().clock()>65){
        fprintf(resFile,"packet_count:%d\n",packet_count);

    }

    //送信者と時刻を記録
    sender[my_addr()][sendercount[my_addr()]] = ph->addr();
    recvtime[my_addr()][sendercount[my_addr()]] = Scheduler::instance().clock();
    //パケット種別に応じて重みづけ
    if(ph->pkttype_==PKT_NORMAL){
        packetweight[my_addr()][sendercount[my_addr()]]=1;
    }
    else if(ph->pkttype_==PKT_CODED){
        packetweight[my_addr()][sendercount[my_addr()]]=CODE_NUM;
    }
    else{
        packetweight[my_addr()][sendercount[my_addr()]]=1;
    }
    
    if (my_addr() == 1) {
        fprintf(historyFile, "%f\t%d\n", recvtime[my_addr()][sendercount[my_addr()]],
                sender[my_addr()][sendercount[my_addr()]]);
    }

    //トポロジ値を算出
    //区切り時間を計算
    float time_limit = 0;
    time_limit = recvtime[my_addr()][sendercount[my_addr()]] - TIME_TH;
    if (time_limit < 0) {
        time_limit = 0;
    }
    fprintf(stdout, "%f - TH = time lim:%f\n", recvtime[my_addr()][sendercount[my_addr()]], time_limit);

    for (int i = 0; i < 10; i++) {
        fprintf(stdout, "%d ", sender[my_addr()][i]);
    }
    fprintf(stdout, "\n");

    topocount[my_addr()] = 0;
    int bunbo = 0;
    fprintf(stdout, "sender:%d sendercount:%d recvtime:%f\n", sender[my_addr()][sendercount[my_addr()]],
            sendercount[my_addr()], recvtime[my_addr()][sendercount[my_addr()]]);
    //受信履歴をファイルに出力
    fprintf(recvlogFile, "node,sendercount,sender,recvtime\n");
    for (int i = 0; i <= sendercount[my_addr()]; i++) {
        fprintf(recvlogFile, "%d,%d,%d,%f\n", my_addr(), i, sender[my_addr()][i], recvtime[my_addr()][i]);
    }
    //区切り時間のエントリ番号を検索
    for (int i = sendercount[my_addr()]; recvtime[my_addr()][i] >= time_limit; i--) {
        topocount[my_addr()] = i;
        //bunbo++;
        bunbo=bunbo+packetweight[my_addr()][i];
        //fprintf(stdout,"for:%d\n",i);
    }
    fprintf(stdout, "区切りエントリはtopocount %d\n", topocount[my_addr()]);
    //区切りエントリに基づいて、現在のエントリから遡り、ヒット数を計測
    hitcount[my_addr()] = 0;
    for (int i = sendercount[my_addr()]; i >= topocount[my_addr()]; i--) {
        if (sender[my_addr()][i] == ph->addr()) {
            //hitcount[my_addr()]++;
            hitcount[my_addr()]=hitcount[my_addr()]+packetweight[my_addr()][i];
            //fprintf(stdout,"time:%f\n",recvtime[my_addr()][i]);
        }
    }
    float topo_all, topo_latest;
    topo_all = (float) hitcount[my_addr()] / (float) bunbo;
    fprintf(stdout, "topo_all:%f (%d/%d)\n", topo_all, hitcount[my_addr()], bunbo);
    fprintf(stdout, "hit:%d\n", hitcount[my_addr()]);
    //ここまでall

    //半分時間補正
    //allエントリ数をkosuuへ
    int kosuu;
    kosuu = sendercount[my_addr()] - topocount[my_addr()] + 1;
    //allの半分のうち、後半の最初のエントリをhalf_countへ
    int half_count;
    half_count = sendercount[my_addr()] - kosuu / 2 + 1;

    fprintf(stdout, "%d-%d+1=hanbun:%d\n", sendercount[my_addr()], topocount[my_addr()], kosuu);
    fprintf(stdout, "half_count:%d\n", half_count);
    //半分のヒット数を計測
    int half_hit = 0;
    for (int i = sendercount[my_addr()]; i >= half_count; i--) {
        if (sender[my_addr()][i] == ph->addr()) {
            half_hit++;
        }
    }
    float half_topo;
    half_topo = (float) half_hit / (float) ((kosuu / 2) + 1);
    //全体のエントリ数が1のときの例外
    if (kosuu == 1) {
        half_topo = 1;
        fprintf(stdout, "全体のエントリ数が1なのでhalf_topoを1に変更\n");
    }
    fprintf(stdout, "半分 %f,%d,%d\n", half_topo, half_hit, ((kosuu / 2) + 1));

    //履歴印字テスト
    fprintf(tempFile,"%d %d %d sender\t",my_addr(),hitcount[my_addr()],bunbo);
    for(int i=sendercount[my_addr()];0<=i;i--){
        fprintf(tempFile,"%d ",sender[my_addr()][i]);
    }
    fprintf(tempFile,"\n");
    fprintf(tempFile,"%d %d %d time\t",my_addr(),hitcount[my_addr()],bunbo);
    for(int i=sendercount[my_addr()];0<=i;i--){
        fprintf(tempFile,"%.4f ",recvtime[my_addr()][i]);
    }
    fprintf(tempFile,"\n");



    //全体+half
    float goukei_topo;
    goukei_topo = (topo_all + half_topo) / 2;
    fprintf(stdout, "合算topo:%f\n", goukei_topo);
    //直近時間補正
    //未実装
    //トポロジ値計算ここまで
    fprintf(stdout,"-------------------送信者のトポロジ値計算終了\n");
    int neighbor_count = 0;
    int ni;
    //隣接ノード数カウント
    for (int i = 0; i < BUF; i++) {
        for (int j = sendercount[my_addr()]; j >= topocount[my_addr()]; j--) {
            if (sender[my_addr()][j] == i) {
                neighbor_count++;
                break;
            }
        }
    }
/*
    for(int i=sendercount[my_addr()];i>=topocount[my_addr()];i--){
        for(ni=0;ni<BUF;ni++){
           if(sender[my_addr()][i]==ni){
               fprintf(mytraceFile,"sender;%d ni:%d\n",sender[my_addr()][i],ni);
               neighbor_count++;
               break;
           }
        }
    }*/
    //fprintf(mytraceFile,"neigNUM\tnode:%d\tnei:%d\n",my_addr(),neighbor_count);
    //隣接ノードの状況把握
    //自ノードはノード1つずつ履歴を確認していく
    int fl_count = 0, nc_count = 0;
    float neigh_topo=0;

    //一度受信したパケットは調査しない
    int aa=0;
    if (recvlog[my_addr()][ph->pktnum_] == 1) {
        aa = 1;
    }
    if(aa==0) {
        for (int neinode = 0; neinode < NODE_NUM; neinode++) {
            fprintf(stdout, "-------------calcing %d node\n", neinode);
            //区切りエントリに基づいて、現在のエントリから遡り、ヒット数を計測
            hitcount[my_addr()] = 0;
            for (int i = sendercount[my_addr()]; i >= topocount[my_addr()]; i--) {
                if (sender[my_addr()][i] == neinode) {
                    hitcount[my_addr()]++;
                    //fprintf(stdout,"time:%f\n",recvtime[my_addr()][i]);
                }
            }
            float topo_all, topo_latest;
            topo_all = (float) hitcount[my_addr()] / (float) bunbo;//neinodeの全体トポロジ値
            fprintf(stdout, "topo_all:%f (%d/%d)\n", topo_all, hitcount[my_addr()], bunbo);
            fprintf(stdout, "hit:%d\n", hitcount[my_addr()]);
            int active=1;//履歴にヒットしない場合は状態カウントしない
            if (hitcount[my_addr()] == 0) {
                fprintf(stdout, "node%dは自ノード(%d)または非隣接ノードのため無効\n", neinode, my_addr());
                active=0;
                //break;
            }
            //ここまでall

            //半分時間補正
            //allエントリ数をkosuuへ
            int kosuu;
            kosuu = sendercount[my_addr()] - topocount[my_addr()] + 1;
            //allの半分のうち、後半の最初のエントリをhalf_countへ
            int half_count;
            half_count = sendercount[my_addr()] - kosuu / 2 + 1;

            fprintf(stdout, "%d-%d+1=hanbun:%d\n", sendercount[my_addr()], topocount[my_addr()], kosuu);
            fprintf(stdout, "half_count:%d\n", half_count);
            //半分のヒット数を計測
            int half_hit = 0;
            for (int i = sendercount[my_addr()]; i >= half_count; i--) {
                if (sender[my_addr()][i] == neinode) {
                    half_hit++;
                }
            }
            float half_topo;
            half_topo = (float) half_hit / (float) ((kosuu / 2) + 1);
            //全体のエントリ数が1のときの例外
            if (kosuu == 1) {
                half_topo = 1;
                fprintf(stdout, "全体のエントリ数が1なのでhalf_topoを1に変更\n");
            }
            fprintf(stdout, "半分 %f,%d,%d\n", half_topo, half_hit, ((kosuu / 2) + 1));

            //全体+half
            float goukei_topo_temp;
            goukei_topo_temp = (topo_all + half_topo) / 2;

            fprintf(stdout, "合算topo:%f\n", goukei_topo_temp);

            if(active==1) {
                if (goukei_topo_temp <= SWITCH_TH) {
                    fl_count++;
                    fprintf(stdout,"node:%d is dinamic\n",neinode);
                } else if (goukei_topo_temp >= SWITCH_TH) {
                    nc_count++;
                    fprintf(stdout,"node:%d is static\n",neinode);

                } else {
                    fprintf(mytraceFile, "err \n");
                    return;
                }
            }
            else if(active==0){
                fprintf(stdout,"node:%d is no entry\n",neinode);
            }
            fprintf(stdout, "*node:%d neighbor node  fl:%d, nc:%d\n", my_addr(), fl_count, nc_count);

            //fprintf(mytraceFile, "node:%d neighbor node %d fl:%d, nc:%d\n",my_addr(),neinode, fl_count, nc_count);
            if(active==1) {
                //fprintf(tpFile, "node:%d\tcalc:%d\tgoukei_topo:%f\ttemp:%f\ttopo_all:%f\thalftopo:%f\tfl:%d\tnc:%d\tneicount:%d\n", my_addr(), neinode,
                //        goukei_topo, goukei_topo_temp,topo_all,half_topo,fl_count, nc_count,neighbor_count);
            }
        }
        //fprintf(mytraceFile, "*node:%d neighbor node  fl:%d, nc:%d\n", my_addr(), fl_count, nc_count);
        fprintf(stdout, "*node:%d neighbor node  fl:%d, nc:%d\n", my_addr(), fl_count, nc_count);

        //  if(fl_count!=0||nc_count!=0)
        fprintf(stdout, "**node:%d neighbor node  fl:%d, nc:%d\n", my_addr(), fl_count, nc_count);
        neigh_topo=(float)fl_count/(float)(fl_count+nc_count);
        fprintf(stdout,"++++++++++++++++++nei:%f\n",neigh_topo);
        //fprintf(tpFile,"node:%d\tfl_count:%d\tnc_count:%d\tNei_topo:%f\tgoukei:%f\n",my_addr(),fl_count,nc_count,neigh_topo,goukei_topo);
        fprintf(tpFile,"%f\t%d\t%d\t%f\t%d\t%d\n",Scheduler::instance().clock(),my_addr(),sender[my_addr()][sendercount[my_addr()]],neigh_topo,fl_count,nc_count);

    }
    sendercount[my_addr()]++;
    //ここここここ

    //ステータス決定
    if (TRANSTH_TYPE == 0){//旧方式
        //トポロジ値による自分のステータス決定
        if (mystatus[my_addr()] == STA_CODEWAIT) {
            mystatus[my_addr()] = STA_CODEWAIT;
        } else if (mystatus[my_addr()] == STA_CODESENDREADY) {
            mystatus[my_addr()] = STA_CODESENDREADY;
        } else if (neighbor_count < NEIGHBOR_TH) {//隣接ノード数が足りない場合強制フラッディング
            mystatus[my_addr()] = STA_FL;
        } else {

            if (goukei_topo <= SWITCH_TH) {
                mystatus[my_addr()] = STA_FL;

            } else if (goukei_topo >= SWITCH_TH) {
                mystatus[my_addr()] = STA_CODEWAIT;

            } else {
                fprintf(mytraceFile, "err define my status\n");
                return;
            }
        }
    }
    else if(TRANSTH_TYPE==1){//ハイブリッド
        //隣接ノードの判定総合結果による自分のステータス決定
        if(mystatus[my_addr()]==STA_CODEWAIT){
            mystatus[my_addr()]=STA_CODEWAIT;
        }
        else if(mystatus[my_addr()]==STA_CODESENDREADY){
            mystatus[my_addr()]=STA_CODESENDREADY;
        }
        else if(neighbor_count<NEIGHBOR_TH){//隣接ノード数が足りない場合強制フラッディング
            mystatus[my_addr()]=STA_FL;
        }
        else {
            if(neigh_topo<=UNDER_TH){//動的判定
                mystatus[my_addr()]=STA_FL;
            }
            else if(UNDER_TH<neigh_topo && neigh_topo<UPPER_TH){//ハイブリッド判定
                mystatus[my_addr()]=STA_HYBRID;
            }
            else if(UPPER_TH<=neigh_topo){//静的判定
                mystatus[my_addr()]=STA_CODEWAIT;

            }
            else{
                fprintf(mytraceFile,"err define my status\n");
                return;
            }
        }
    }
    else if(TRANSTH_TYPE==2){//隣接ノード総合判定方式
        //隣接ノードの判定総合結果による自分のステータス決定
        if(mystatus[my_addr()]==STA_CODEWAIT){
            mystatus[my_addr()]=STA_CODEWAIT;
        }
        else if(mystatus[my_addr()]==STA_CODESENDREADY){
            mystatus[my_addr()]=STA_CODESENDREADY;
        }
        else if(neighbor_count<NEIGHBOR_TH){//隣接ノード数が足りない場合強制フラッディング
            mystatus[my_addr()]=STA_FL;
            fprintf(mytraceFile,"強制フラッディング\n");
        }
        else {
            if(nc_count<=fl_count){
                mystatus[my_addr()]=STA_FL;
            }
            else if(fl_count<nc_count){
                mystatus[my_addr()]=STA_CODEWAIT;
            }
            else{
                fprintf(mytraceFile,"err define my status\n");
                return;
            }
        }
    }


    //fprintf(mytraceFile,"TOPO node:%d %d topo:%f\n",my_addr(),mystatus[my_addr()],goukei_topo);
	//受信(転送)動作
	//NORMALパケット
	if(ph->pkttype_==PKT_NORMAL) {
		//一度受信したパケットは破棄
		if (recvlog[my_addr()][ph->pktnum_] == 1) {
			//fprintf(mytraceFile, "Drop\t%f node:%d from:%d type:%d pktNo:%d \n", Scheduler::instance().clock(), my_addr(),ph->addr(),ph->pkttype_, ph->pktnum_);
			return;
		}
        if(Scheduler::instance().clock()>30 ) {
            packet_count++;
        }
		//受信記録
		recvlog[my_addr()][ph->pktnum_] = 1;
        fprintf(mytraceFile, "r\t%f\tnode:%d\tfrom:%d\ttype:%d\tpktNo:%d\tcv:%d\ttopo:%f\tstatus:%d\tnei:%d\tfl:%d\tnc:%d\n", Scheduler::instance().clock(), my_addr(),ph->addr(),ph->pkttype_, ph->pktnum_,ph->codevc_,goukei_topo,mystatus[my_addr()],neighbor_count,fl_count,nc_count);

        if(my_addr()==1){//デバッグ用
            for(int i=1;i<12;i++) {
                fprintf(stdout, "%d",recvlog[my_addr()][i]);
            }
            fprintf(stdout,"\n");
        }

	}
	//符号パケット
	else if(ph->pkttype_==PKT_CODED){
	    //一度受信した符号パケットは破棄(NORMALとは管理方法が異なる)
        for(int i=0;i<BUF;i++){
            if(recvcodelog[my_addr()][i]==ph->pktnum_){
                if(ph->codenum_==2) {
                    if(recvcode1[my_addr()][recvcodecount[my_addr()]] == ph->pkt1_ &&
                       recvcode2[my_addr()][recvcodecount[my_addr()]] == ph->pkt2_){
                        return;
                    }

                }
                else if(ph->codenum_==3) {
                    if(recvcode1[my_addr()][recvcodecount[my_addr()]] == ph->pkt1_ &&
                       recvcode2[my_addr()][recvcodecount[my_addr()]] == ph->pkt2_ &&
                       recvcode3[my_addr()][recvcodecount[my_addr()]] == ph->pkt3_){
                        return;
                    }

                }
                return;
            }
        }
/*		if (recvcodelog[my_addr()][ph->pktnum_] == 1) {
			fprintf(mytraceFile, "Err \t%f node:%d from:%d type:%d pktNo:%d \n", Scheduler::instance().clock(), my_addr(),ph->addr(),ph->pkttype_, ph->pktnum_);
			return;
		}*/
        if(Scheduler::instance().clock()>30 ) {
            packet_count++;
        }
    	//受信記録
		recvcodelog[my_addr()][recvcodecount[my_addr()]] = ph->pktnum_;
		recvcodevec[my_addr()][recvcodecount[my_addr()]] = ph->codevc_;
		if(ph->codenum_==2) {
			recvcode1[my_addr()][recvcodecount[my_addr()]] = ph->pkt1_;
			recvcode2[my_addr()][recvcodecount[my_addr()]] = ph->pkt2_;

		}
		else if(ph->codenum_==3) {
			recvcode1[my_addr()][recvcodecount[my_addr()]] = ph->pkt1_;
			recvcode2[my_addr()][recvcodecount[my_addr()]] = ph->pkt2_;
			recvcode3[my_addr()][recvcodecount[my_addr()]] = ph->pkt3_;

		}
		else if(ph->codenum_==4) {
			recvcode1[my_addr()][recvcodecount[my_addr()]] = ph->pkt1_;
			recvcode2[my_addr()][recvcodecount[my_addr()]] = ph->pkt2_;
			recvcode3[my_addr()][recvcodecount[my_addr()]] = ph->pkt3_;
			recvcode4[my_addr()][recvcodecount[my_addr()]] = ph->pkt4_;
		}
		else if(ph->codenum_==5) {
			recvcode1[my_addr()][recvcodecount[my_addr()]] = ph->pkt1_;
			recvcode2[my_addr()][recvcodecount[my_addr()]] = ph->pkt2_;
			recvcode3[my_addr()][recvcodecount[my_addr()]] = ph->pkt3_;
			recvcode4[my_addr()][recvcodecount[my_addr()]] = ph->pkt4_;
			recvcode5[my_addr()][recvcodecount[my_addr()]] = ph->pkt5_;
		}
		fprintf(mytraceFile, "rc\t%f\tnode:%d\tfrom:%d\ttype:%d\tpktNo:%d\tcv:%d\ttopo:%f\tstatus:%d\tnei:%d\tenc_count:%d\t[ %d %d %d %d %d]\n",
				Scheduler::instance().clock(), my_addr(),ph->addr(),ph->pkttype_, ph->pktnum_,ph->codevc_,goukei_topo,mystatus[my_addr()],neighbor_count,ph->encode_count_,ph->pkt1_,ph->pkt2_,ph->pkt3_,ph->pkt4_,ph->pkt5_);


        //受信記録ここまで

		if(my_addr()==1) {
			fprintf(codelogFile, "%d\t", recvcodecount[my_addr()]);
			fprintf(codelogFile, "%d\t", recvcodelog[my_addr()][recvcodecount[my_addr()]]);
			fprintf(codelogFile, "%d\t", recvcodevec[my_addr()][recvcodecount[my_addr()]]);
			fprintf(codelogFile, "%d\t", recvcode1[my_addr()][recvcodecount[my_addr()]]);
			fprintf(codelogFile, "%d\t", recvcode2[my_addr()][recvcodecount[my_addr()]]);
			fprintf(codelogFile, "%d\t", recvcode3[my_addr()][recvcodecount[my_addr()]]);
			fprintf(codelogFile, "%d\t", recvcode4[my_addr()][recvcodecount[my_addr()]]);
			fprintf(codelogFile, "%d\t", recvcode5[my_addr()][recvcodecount[my_addr()]]);
			fprintf(codelogFile, "%d\n", recvcodeFlag[my_addr()][recvcodecount[my_addr()]]);

		}
		if(my_addr()==2) {
			fprintf(codelogFile2, "%d\t", recvcodecount[my_addr()]);
			fprintf(codelogFile2, "%d\t", recvcodelog[my_addr()][recvcodecount[my_addr()]]);
			fprintf(codelogFile2, "%d\t", recvcodevec[my_addr()][recvcodecount[my_addr()]]);
			fprintf(codelogFile2, "%d\t", recvcode1[my_addr()][recvcodecount[my_addr()]]);
			fprintf(codelogFile2, "%d\t", recvcode2[my_addr()][recvcodecount[my_addr()]]);
			fprintf(codelogFile2, "%d\t", recvcode3[my_addr()][recvcodecount[my_addr()]]);
			fprintf(codelogFile2, "%d\t", recvcode4[my_addr()][recvcodecount[my_addr()]]);
			fprintf(codelogFile2, "%d\t", recvcode5[my_addr()][recvcodecount[my_addr()]]);
			fprintf(codelogFile2, "%d\n", recvcodeFlag[my_addr()][recvcodecount[my_addr()]]);
		}
		//dcwaitの復号チェック
		if(ph->codenum_==2){

		}
		else if(ph->codenum_==3){

		}


		//受信パケットの復号
		if(ph->codenum_==2){

			for(int i=0; i<recvcodecount[my_addr()];i++){//符号＋符号
				if((recvcode1[my_addr()][i]==ph->pkt1_ && recvcode2[my_addr()][i]==ph->pkt2_ && recvcodevec[my_addr()][i]!=ph->codevc_)||
				   (recvcode1[my_addr()][i]==ph->pkt2_ && recvcode2[my_addr()][i]==ph->pkt1_ && recvcodevec[my_addr()][i]!=ph->codevc_)) {
					fprintf(mytraceFile, "decodeC\t%f\tnode:%d\tfrom:%d\ttype:%d\tpktNo:%d\tcv:%d\ttopo:%f\tstatus:%d\tnei:%d\t[ %d %d %d %d %d]\n",
							Scheduler::instance().clock(), my_addr(),ph->addr(),ph->pkttype_, ph->pktnum_,ph->codevc_,goukei_topo,mystatus[my_addr()],neighbor_count,ph->pkt1_,ph->pkt2_,ph->pkt3_,ph->pkt4_,ph->pkt5_);
					recvlog[my_addr()][ph->pkt1_]=1;
					recvlog[my_addr()][ph->pkt2_]=1;
					fprintf(mytraceFile, "g\t%f\tnode:%d\tfrom:%d\ttype:N\tpktNo:%d\tcv:%d\ttopo:%f\tstatus:%d\tnei:%d\t[ %d %d %d %d %d]\n",
							Scheduler::instance().clock(), my_addr(),ph->addr(), ph->pkt1_,ph->codevc_,goukei_topo,mystatus[my_addr()],neighbor_count,ph->pkt1_,ph->pkt2_,ph->pkt3_,ph->pkt4_,ph->pkt5_);
					fprintf(mytraceFile, "g\t%f\tnode:%d\tfrom:%d\ttype:N\tpktNo:%d\tcv:%d\ttopo:%f\tstatus:%d\tnei:%d\t[ %d %d %d %d %d]\n",
							Scheduler::instance().clock(), my_addr(),ph->addr(), ph->pkt2_,ph->codevc_,goukei_topo,mystatus[my_addr()],neighbor_count,ph->pkt1_,ph->pkt2_,ph->pkt3_,ph->pkt4_,ph->pkt5_);

					recvcodeFlag[my_addr()][recvcodecount[my_addr()]]=1;
					flag=1;

					ph->addr() = my_addr();
					ph->pkttype_ = PKT_CODED;
					ch->next_hop() = IP_BROADCAST;
					ch->addr_type() = NS_AF_NONE;
					ch->direction() = hdr_cmn::DOWN;

					send(p,0);
					break;
				}
			}
			if(flag!=1&&(recvlog[my_addr()][ph->pkt1_]==1 || recvlog[my_addr()][ph->pkt2_]==1)){//通常＋符号
				fprintf(mytraceFile, "decodeN\t%f\tnode:%d\tfrom:%d\ttype:%d\tpktNo:%d\tcv:%d\ttopo:%f\tstatus:%d\tnei:%d\t[ %d %d %d %d %d]\n",
						Scheduler::instance().clock(), my_addr(),ph->addr(),ph->pkttype_, ph->pktnum_,ph->codevc_,goukei_topo,mystatus[my_addr()],neighbor_count,ph->pkt1_,ph->pkt2_,ph->pkt3_,ph->pkt4_,ph->pkt5_);
				recvlog[my_addr()][ph->pkt1_]=1;
				recvlog[my_addr()][ph->pkt2_]=1;
				fprintf(mytraceFile, "g\t%f\tnode:%d\tfrom:%d\ttype:N\tpktNo:%d\tcv:%d\ttopo:%f\tstatus:%d\tnei:%d\t[ %d %d %d %d %d]\n",
						Scheduler::instance().clock(), my_addr(),ph->addr(), ph->pkt1_,ph->codevc_,goukei_topo,mystatus[my_addr()],neighbor_count,ph->pkt1_,ph->pkt2_,ph->pkt3_,ph->pkt4_,ph->pkt5_);
				fprintf(mytraceFile, "g\t%f\tnode:%d\tfrom:%d\ttype:N\tpktNo:%d\tcv:%d\ttopo:%f\tstatus:%d\tnei:%d\t[ %d %d %d %d %d]\n",
						Scheduler::instance().clock(), my_addr(),ph->addr(), ph->pkt2_,ph->codevc_,goukei_topo,mystatus[my_addr()],neighbor_count,ph->pkt1_,ph->pkt2_,ph->pkt3_,ph->pkt4_,ph->pkt5_);

				recvcodeFlag[my_addr()][recvcodecount[my_addr()]]=1;
				flag=1;
			}
			if(flag==0){//復号できない場合
				fprintf(mytraceFile, "failDC\t%f\tnode:%d\tfrom:%d\ttype:%d\tpktNo:%d\tcv:%d\ttopo:%f\tstatus:%d\tnei:%d\t[ %d %d %d %d %d]\n",
						Scheduler::instance().clock(), my_addr(),ph->addr(),ph->pkttype_, ph->pktnum_,ph->codevc_,goukei_topo,mystatus[my_addr()],neighbor_count,ph->pkt1_,ph->pkt2_,ph->pkt3_,ph->pkt4_,ph->pkt5_);
				dcwait1[my_addr()][decodequeuecount[my_addr()]]=ph->pkt1_;
				dcwait2[my_addr()][decodequeuecount[my_addr()]]=ph->pkt2_;
				dcwaitvec[my_addr()][decodequeuecount[my_addr()]]=ph->codevc_;
				decodequeuecount[my_addr()]++;
				recvcodecount[my_addr()]++;
				return;
			}
			recvcodecount[my_addr()]++;
		}
		else if(ph->codenum_==3){
			for(int i=0; i<recvcodecount[my_addr()];i++) {//符号＋符号+符号
				if (((recvcode1[my_addr()][i] == ph->pkt1_ && recvcode2[my_addr()][i] == ph->pkt2_ &&
					  recvcode3[my_addr()][i] == ph->pkt3_) ||
					 (recvcode1[my_addr()][i] == ph->pkt1_ && recvcode2[my_addr()][i] == ph->pkt3_ &&
					  recvcode3[my_addr()][i] == ph->pkt2_) ||
					 (recvcode1[my_addr()][i] == ph->pkt2_ && recvcode2[my_addr()][i] == ph->pkt1_ &&
					  recvcode3[my_addr()][i] == ph->pkt3_) ||
					 (recvcode1[my_addr()][i] == ph->pkt2_ && recvcode2[my_addr()][i] == ph->pkt3_ &&
					  recvcode3[my_addr()][i] == ph->pkt1_) ||
					 (recvcode1[my_addr()][i] == ph->pkt3_ && recvcode2[my_addr()][i] == ph->pkt1_ &&
					  recvcode3[my_addr()][i] == ph->pkt2_) ||
					 (recvcode1[my_addr()][i] == ph->pkt3_ && recvcode2[my_addr()][i] == ph->pkt2_ &&
					  recvcode3[my_addr()][i] == ph->pkt1_)) &&
					(recvcodevec[my_addr()][i] != ph->codevc_)) {
					for (int j = i + 1; j < recvcodecount[my_addr()]; j++) {
						if (((recvcode1[my_addr()][j] == ph->pkt1_ && recvcode2[my_addr()][j] == ph->pkt2_ &&
							  recvcode3[my_addr()][j] == ph->pkt3_) ||
							 (recvcode1[my_addr()][j] == ph->pkt1_ && recvcode2[my_addr()][j] == ph->pkt3_ &&
							  recvcode3[my_addr()][j] == ph->pkt2_) ||
							 (recvcode1[my_addr()][j] == ph->pkt2_ && recvcode2[my_addr()][j] == ph->pkt1_ &&
							  recvcode3[my_addr()][j] == ph->pkt3_) ||
							 (recvcode1[my_addr()][j] == ph->pkt2_ && recvcode2[my_addr()][j] == ph->pkt3_ &&
							  recvcode3[my_addr()][j] == ph->pkt1_) ||
							 (recvcode1[my_addr()][j] == ph->pkt3_ && recvcode2[my_addr()][j] == ph->pkt1_ &&
							  recvcode3[my_addr()][j] == ph->pkt2_) ||
							 (recvcode1[my_addr()][j] == ph->pkt3_ && recvcode2[my_addr()][j] == ph->pkt2_ &&
							  recvcode3[my_addr()][j] == ph->pkt1_)) &&
							((recvcodevec[my_addr()][j] != ph->codevc_) &&
							 (recvcodevec[my_addr()][i] != recvcodevec[my_addr()][j]))) {

							fprintf(mytraceFile, "decodeCC\t%f\tnode:%d\tfrom:%d\ttype:%d\tpktNo:%d\tcv:%d\ttopo:%f\tstatus:%d\tnei:%d\t[ %d %d %d %d %d]\n",
									Scheduler::instance().clock(), my_addr(),ph->addr(),ph->pkttype_, ph->pktnum_,ph->codevc_,goukei_topo,mystatus[my_addr()],neighbor_count,ph->pkt1_,ph->pkt2_,ph->pkt3_,ph->pkt4_,ph->pkt5_);
							recvlog[my_addr()][ph->pkt1_] = 1;
							recvlog[my_addr()][ph->pkt2_] = 1;
							recvlog[my_addr()][ph->pkt3_] = 1;
							fprintf(mytraceFile, "g\t%f\tnode:%d\tfrom:%d\ttype:N\tpktNo:%d\tcv:%d\ttopo:%f\tstatus:%d\tnei:%d\t[ %d %d %d %d %d]\n",
									Scheduler::instance().clock(), my_addr(),ph->addr(), ph->pkt1_,ph->codevc_,goukei_topo,mystatus[my_addr()],neighbor_count,ph->pkt1_,ph->pkt2_,ph->pkt3_,ph->pkt4_,ph->pkt5_);
							fprintf(mytraceFile, "g\t%f\tnode:%d\tfrom:%d\ttype:N\tpktNo:%d\tcv:%d\ttopo:%f\tstatus:%d\tnei:%d\t[ %d %d %d %d %d]\n",
									Scheduler::instance().clock(), my_addr(),ph->addr(), ph->pkt2_,ph->codevc_,goukei_topo,mystatus[my_addr()],neighbor_count,ph->pkt1_,ph->pkt2_,ph->pkt3_,ph->pkt4_,ph->pkt5_);
							fprintf(mytraceFile, "g\t%f\tnode:%d\tfrom:%d\ttype:N\tpktNo:%d\tcv:%d\ttopo:%f\tstatus:%d\tnei:%d\t[ %d %d %d %d %d]\n",
									Scheduler::instance().clock(), my_addr(),ph->addr(), ph->pkt3_,ph->codevc_,goukei_topo,mystatus[my_addr()],neighbor_count,ph->pkt1_,ph->pkt2_,ph->pkt3_,ph->pkt4_,ph->pkt5_);


							recvcodeFlag[my_addr()][recvcodecount[my_addr()]] = 1;
							flag = 1;
							if (my_addr() == 2) {
								fprintf(codelogFile2, "%d\t",
										recvcodecount[my_addr()]);
								fprintf(codelogFile2, "%d\t", recvcodelog[my_addr()][recvcodecount[my_addr()]]);
								fprintf(codelogFile2, "%d\t", recvcodevec[my_addr()][recvcodecount[my_addr()]]);
								fprintf(codelogFile2, "%d\t", recvcode1[my_addr()][recvcodecount[my_addr()]]);
								fprintf(codelogFile2, "%d\t", recvcode2[my_addr()][recvcodecount[my_addr()]]);
								fprintf(codelogFile2, "%d\t", recvcode3[my_addr()][recvcodecount[my_addr()]]);
								fprintf(codelogFile2, "%d\t", recvcode4[my_addr()][recvcodecount[my_addr()]]);
								fprintf(codelogFile2, "%d\t", recvcode5[my_addr()][recvcodecount[my_addr()]]);
								fprintf(codelogFile2, "%d\n", recvcodeFlag[my_addr()][recvcodecount[my_addr()]]);
							}
							break;
						}
					}
				}
			}
			for(int i=0; i<recvcodecount[my_addr()];i++){//符号＋符号+通常
				if(((recvcode1[my_addr()][i]==ph->pkt1_ && recvcode2[my_addr()][i]==ph->pkt2_ && recvcode3[my_addr()][i]==ph->pkt3_)||
					(recvcode1[my_addr()][i]==ph->pkt1_ && recvcode2[my_addr()][i]==ph->pkt3_ && recvcode3[my_addr()][i]==ph->pkt2_)||
					(recvcode1[my_addr()][i]==ph->pkt2_ && recvcode2[my_addr()][i]==ph->pkt1_ && recvcode3[my_addr()][i]==ph->pkt3_)||
					(recvcode1[my_addr()][i]==ph->pkt2_ && recvcode2[my_addr()][i]==ph->pkt3_ && recvcode3[my_addr()][i]==ph->pkt1_)||
					(recvcode1[my_addr()][i]==ph->pkt3_ && recvcode2[my_addr()][i]==ph->pkt1_ && recvcode3[my_addr()][i]==ph->pkt2_)||
					(recvcode1[my_addr()][i]==ph->pkt3_ && recvcode2[my_addr()][i]==ph->pkt2_ && recvcode3[my_addr()][i]==ph->pkt1_))&&
				   (recvcodevec[my_addr()][i]!=ph->codevc_)&&(flag!=1)){
					if(recvlog[my_addr()][ph->pkt1_]==1||recvlog[my_addr()][ph->pkt2_]==1||recvlog[my_addr()][ph->pkt3_]==1){
						fprintf(mytraceFile, "decodeCN\t%f\tnode:%d\tfrom:%d\ttype:%d\tpktNo:%d\tcv:%d\ttopo:%f\tstatus:%d\tnei:%d\t[ %d %d %d %d %d]\n",
								Scheduler::instance().clock(), my_addr(),ph->addr(),ph->pkttype_, ph->pktnum_,ph->codevc_,goukei_topo,mystatus[my_addr()],neighbor_count,ph->pkt1_,ph->pkt2_,ph->pkt3_,ph->pkt4_,ph->pkt5_);
						fprintf(stdout,"@@@%d %d %d\n",recvlog[my_addr()][ph->pkt1_],recvlog[my_addr()][ph->pkt2_],recvlog[my_addr()][ph->pkt3_]);

						recvlog[my_addr()][ph->pkt1_] = 1;
						recvlog[my_addr()][ph->pkt2_] = 1;
						recvlog[my_addr()][ph->pkt3_] = 1;
						fprintf(mytraceFile, "g\t%f\tnode:%d\tfrom:%d\ttype:N\tpktNo:%d\tcv:%d\ttopo:%f\tstatus:%d\tnei:%d\t[ %d %d %d %d %d]\n",
								Scheduler::instance().clock(), my_addr(),ph->addr(), ph->pkt1_,ph->codevc_,goukei_topo,mystatus[my_addr()],neighbor_count,ph->pkt1_,ph->pkt2_,ph->pkt3_,ph->pkt4_,ph->pkt5_);
						fprintf(mytraceFile, "g\t%f\tnode:%d\tfrom:%d\ttype:N\tpktNo:%d\tcv:%d\ttopo:%f\tstatus:%d\tnei:%d\t[ %d %d %d %d %d]\n",
								Scheduler::instance().clock(), my_addr(),ph->addr(), ph->pkt2_,ph->codevc_,goukei_topo,mystatus[my_addr()],neighbor_count,ph->pkt1_,ph->pkt2_,ph->pkt3_,ph->pkt4_,ph->pkt5_);
						fprintf(mytraceFile, "g\t%f\tnode:%d\tfrom:%d\ttype:N\tpktNo:%d\tcv:%d\ttopo:%f\tstatus:%d\tnei:%d\t[ %d %d %d %d %d]\n",
								Scheduler::instance().clock(), my_addr(),ph->addr(), ph->pkt3_,ph->codevc_,goukei_topo,mystatus[my_addr()],neighbor_count,ph->pkt1_,ph->pkt2_,ph->pkt3_,ph->pkt4_,ph->pkt5_);


						recvcodeFlag[my_addr()][recvcodecount[my_addr()]] = 1;
						flag = 1;
						break;
					}
				}
			}

			//全部ノーマルパケット持ってた
			if(((recvlog[my_addr()][ph->pkt1_]==1 && recvlog[my_addr()][ph->pkt2_]==1)||
				(recvlog[my_addr()][ph->pkt1_]==1 && recvlog[my_addr()][ph->pkt3_]==1)||
				(recvlog[my_addr()][ph->pkt2_]==1 && recvlog[my_addr()][ph->pkt3_]==1))&&(flag!=1)){//符号+通常＋通常
				fprintf(mytraceFile, "decodeNN\t%f\tnode:%d\tfrom:%d\ttype:%d\tpktNo:%d\tcv:%d\ttopo:%f\tstatus:%d\tnei:%d\t[ %d %d %d %d %d]\n",
						Scheduler::instance().clock(), my_addr(),ph->addr(),ph->pkttype_, ph->pktnum_,ph->codevc_,goukei_topo,mystatus[my_addr()],neighbor_count,ph->pkt1_,ph->pkt2_,ph->pkt3_,ph->pkt4_,ph->pkt5_);
				recvlog[my_addr()][ph->pkt1_] = 1;
				recvlog[my_addr()][ph->pkt2_] = 1;
				recvlog[my_addr()][ph->pkt3_] = 1;
				fprintf(mytraceFile, "g\t%f\tnode:%d\tfrom:%d\ttype:N\tpktNo:%d\tcv:%d\ttopo:%f\tstatus:%d\tnei:%d\t[ %d %d %d %d %d]\n",
						Scheduler::instance().clock(), my_addr(),ph->addr(), ph->pkt1_,ph->codevc_,goukei_topo,mystatus[my_addr()],neighbor_count,ph->pkt1_,ph->pkt2_,ph->pkt3_,ph->pkt4_,ph->pkt5_);
				fprintf(mytraceFile, "g\t%f\tnode:%d\tfrom:%d\ttype:N\tpktNo:%d\tcv:%d\ttopo:%f\tstatus:%d\tnei:%d\t[ %d %d %d %d %d]\n",
						Scheduler::instance().clock(), my_addr(),ph->addr(), ph->pkt2_,ph->codevc_,goukei_topo,mystatus[my_addr()],neighbor_count,ph->pkt1_,ph->pkt2_,ph->pkt3_,ph->pkt4_,ph->pkt5_);
				fprintf(mytraceFile, "g\t%f\tnode:%d\tfrom:%d\ttype:N\tpktNo:%d\tcv:%d\ttopo:%f\tstatus:%d\tnei:%d\t[ %d %d %d %d %d]\n",
						Scheduler::instance().clock(), my_addr(),ph->addr(), ph->pkt3_,ph->codevc_,goukei_topo,mystatus[my_addr()],neighbor_count,ph->pkt1_,ph->pkt2_,ph->pkt3_,ph->pkt4_,ph->pkt5_);


				recvcodeFlag[my_addr()][recvcodecount[my_addr()]] = 1;
				flag = 1;
			}
			if(flag==0){//復号できない場合
				fprintf(mytraceFile, "failDC\t%f\tnode:%d\tfrom:%d\ttype:%d\tpktNo:%d\tcv:%d\ttopo:%f\tstatus:%d\tnei:%d\t[ %d %d %d %d %d]\n",
						Scheduler::instance().clock(), my_addr(),ph->addr(),ph->pkttype_, ph->pktnum_,ph->codevc_,goukei_topo,mystatus[my_addr()],neighbor_count,ph->pkt1_,ph->pkt2_,ph->pkt3_,ph->pkt4_,ph->pkt5_);
				dcwait1[my_addr()][decodequeuecount[my_addr()]]=ph->pkt1_;
				dcwait2[my_addr()][decodequeuecount[my_addr()]]=ph->pkt2_;
				dcwait3[my_addr()][decodequeuecount[my_addr()]]=ph->pkt3_;
				dcwaitvec[my_addr()][decodequeuecount[my_addr()]]=ph->codevc_;
				decodequeuecount[my_addr()]++;

				recvcodecount[my_addr()]++;
				return;
			}
			recvcodecount[my_addr()]++;
		}
		else if(ph->codenum_==4){
			fprintf(mytraceFile,"err:codenum4\n");
		}
		else if(ph->codenum_==5){
			fprintf(mytraceFile,"err:codenum5\n");
		}
		else{
		    fprintf(mytraceFile,"err:decode\n");
		}

	}
	else if(ph->pkttype_==PKT_REQUEST){


	}
	else if(ph->pkttype_==PKT_REPLY){


	}

	//全体復号チェック
	for(int i=0;i<BUF;i++){
		if(recvcodeFlag[my_addr()][i]==1){
            //fprintf(mytraceFile,"なんだこれ%d\n",i);
		}
	}
    //受信記録（重い
            /*
    fprintf(recvhistoryFile,"node:%d\t",my_addr());
    for(int i=1;i<BUF;i++) {
        fprintf(recvhistoryFile,"%d ",recvlog[my_addr()][i]);
    }
    fprintf(recvhistoryFile,"\n");
    */
    //符号化記録
    fprintf(recvcodehistoryFile,"node:%d\n",my_addr());
    for(int i=0;i<recvcodecount[my_addr()];i++){
		fprintf(recvcodehistoryFile, "decodeC\t%f\tnode:%d\tfrom:%d\ttype:%d\tpktNo:%d\tcv:%d\ttopo:%f\tstatus:%d\tnei:%d\t[ %d %d %d %d %d]\n",
				Scheduler::instance().clock(), my_addr(),ph->addr(),ph->pkttype_, ph->pktnum_,ph->codevc_,goukei_topo,mystatus[my_addr()],neighbor_count,ph->pkt1_,ph->pkt2_,ph->pkt3_,ph->pkt4_,ph->pkt5_);

		fprintf(recvcodehistoryFile,"%d [%d][%d %d %d %d %d]\n",i,recvcodevec[my_addr()][i],recvcode1[my_addr()][i],recvcode2[my_addr()][i],recvcode3[my_addr()][i],recvcode4[my_addr()][i],recvcode5[my_addr()][i]);
    }


    //到達率計算
    if(Scheduler::instance().clock()>60&&my_addr()==17){
        int aru_count = 0;
        for (int i = 121; i <= 240; i++) {
            if (recvlog[my_addr()][i] == 1) {
                aru_count++;
            }
        }
        //fprintf(mytraceFile,"node:%d d_rate:%f\n",my_addr(),(float)aru_count/(float)(BUF-1));
        fprintf(resFile, "node:%d d_rate:%f\n", my_addr(), (float) aru_count / 120);

        for(int i=121;i<=240;i++){
            fprintf(resFile,"%d",recvlog[my_addr()][i]);
        }
        fprintf(resFile,"\n");
    }



    //送信動作
	if(ph->pkttype_==PKT_REQUEST){
	}
	else if(ph->pkttype_==PKT_REPLY){
	}
	else if(ph->pkttype_==PKT_CODED){
	    if(flag == 1){//符号パケットを受信して復号に成功した場合
	       /* if(CODE_NUM == 2){
                fprintf(mytraceFile, "fc\t%f\tnode:%d\tfrom:%d\ttype:C\tpktNo:%d\tcv:%d\n", Scheduler::instance().clock(), my_addr(),ph->addr(),ph->pktnum_,ph->codevc_);
                fprintf(stdout,"おはよう\n");
                ph->addr() = my_addr();
                ph->pkttype_ = PKT_CODED;
                ch->next_hop() = IP_BROADCAST;
                ch->addr_type() = NS_AF_NONE;
                ch->direction() = hdr_cmn::DOWN;

                send(p,0);
                return;
	        }*/


           /* if (CODE_NUM == 2) {
                createCodepacket2(collectlist[my_addr()][0], collectlist[my_addr()][1]);

            } else if (CODE_NUM == 3) {
                createCodepacket3(collectlist[my_addr()][0], collectlist[my_addr()][1], collectlist[my_addr()][2]);
                mystatus[my_addr()] = STA_FL;
                collectNum[my_addr()] = 0;
            } else if (CODE_NUM == 4) {
                createCodepacket4(collectlist[my_addr()][0], collectlist[my_addr()][1], collectlist[my_addr()][2],
                                  collectlist[my_addr()][3]);
                mystatus[my_addr()] = STA_FL;
                collectNum[my_addr()] = 0;
            } else if (CODE_NUM == 5) {
                createCodepacket5(collectlist[my_addr()][0], collectlist[my_addr()][1], collectlist[my_addr()][2],
                                  collectlist[my_addr()][3], collectlist[my_addr()][4]);
                mystatus[my_addr()] = STA_FL;
                collectNum[my_addr()] = 0;
            }*/
           fprintf(mytraceFile,"%d < %d\n",ph->encode_count_,CODE_NUM);
           //これでいけるか？
            if(ph->encode_count_<CODE_NUM) {
                if (CODE_NUM == 2) {
                    createCodepacket2(ph->pkt1_, ph->pkt2_, ph->encode_count_);
                    mystatus[my_addr()] = STA_FL;
                    collectNum[my_addr()] = 0;
                    fprintf(mytraceFile, "fc\t%f\tnode:%d\tfrom:%d\ttype:C\tpktNo:%d\tcv:%d\n", Scheduler::instance().clock(), my_addr(),ph->addr(),ph->pktnum_,ph->codevc_);


                } else if (CODE_NUM == 3) {
                    createCodepacket3(ph->pkt1_, ph->pkt2_, ph->pkt3_, ph->encode_count_);
                    mystatus[my_addr()] = STA_FL;
                    collectNum[my_addr()] = 0;
                    fprintf(mytraceFile, "rec\t%f\tnode:%d\tfrom:%d\ttype:C\tpktNo:%d\tcv:%d\n", Scheduler::instance().clock(), my_addr(),ph->addr(),ph->pktnum_,ph->codevc_);
                }

            }
            else{
                fprintf(mytraceFile,"return\n");
                return;
            }
            return;

            //とりあえず中継のみ
        /*    fprintf(mytraceFile, "fc\t%f\tnode:%d\tfrom:%d\ttype:C\tpktNo:%d\tcv:%d\n", Scheduler::instance().clock(), my_addr(),ph->addr(),ph->pktnum_,ph->codevc_);
                ph->hop_count_++;
                ph->addr() = my_addr();
                ph->pkttype_ = PKT_CODED;
                ch->next_hop() = IP_BROADCAST;
                ch->addr_type() = NS_AF_NONE;
                ch->direction() = hdr_cmn::DOWN;
                send(p,0);
                return;*/

        }
	    else if(flag == 0){//復号失敗、復号失敗時にreturnしているのでここには到達しないはず
	        fprintf(mytraceFile,"%d\n",flag);
	    }
	    else{
	        fprintf(mytraceFile,"err Decode flag is wrong\n");
	    }
        //fprintf(mytraceFile,"node:%d flag:%d\n",my_addr(),flag);

		//とりあえず中継のみ
	/*	ph->addr() = my_addr();
		ph->pkttype_ = PKT_CODED;
		ch->next_hop() = IP_BROADCAST;
		ch->addr_type() = NS_AF_NONE;
		ch->direction() = hdr_cmn::DOWN;
		send(p,0);
		return;*/
	}
	else if(ph->pkttype_==PKT_NORMAL) {
		//自分の状態により動作変更
		if(mystatus[my_addr()] == STA_FL){
		    int tempaddr = ph->addr();
			ph->addr() = my_addr();
			ph->pkttype_ = PKT_NORMAL;
			ph->hop_count_++;
			ch->next_hop() = IP_BROADCAST;
			ch->addr_type() = NS_AF_NONE;
			ch->direction() = hdr_cmn::DOWN;
            fprintf(mytraceFile, "f\t%f\tnode:%d\tfrom:%d\ttype:%d\tpktNo:%d\n", Scheduler::instance().clock(), my_addr(),tempaddr,ph->pkttype_ ,ph->pktnum_);

            //即時送信
            send(p,0);
            //遅延送信
            //Scheduler::instance().schedule(target_,p,0.01 * Random::uniform());
			return;
		} else if (mystatus[my_addr()] == STA_CODEWAIT) {//符号パケット収集
			collectlist[my_addr()][collectNum[my_addr()]] = ph->pktnum_;
			collectNum[my_addr()]++;
			fprintf(mytraceFile, "col1\t%f\tnode:%d\tfrom:%d\ttype:%d\tpktNo:%d \n", Scheduler::instance().clock(),
					my_addr(), ph->addr(), ph->pkttype_, ph->pktnum_);
			if(collectNum[my_addr()]+1==CODE_NUM) {//あとひとつあつまれば送信できるとき
				mystatus[my_addr()] = STA_CODESENDREADY;
			}


		} else if (mystatus[my_addr()] == STA_CODESENDREADY) {//収集完了から送信
			collectlist[my_addr()][collectNum[my_addr()]] = ph->pktnum_;
			collectNum[my_addr()]++;
			fprintf(mytraceFile, "col2\t%f\tnode:%d\tfrom:%d\ttype:%d\tpktNo:%d \n", Scheduler::instance().clock(),
					my_addr(), ph->addr(), ph->pkttype_, ph->pktnum_);


			if (collectNum[my_addr()] == CODE_NUM) {
			    //並べ替え処理
			    int temp=-1;
                for(int i=0;i<collectNum[my_addr()];i++){
                    for(int j=collectNum[my_addr()]-1;j>i;j--){
                        if(collectlist[my_addr()][j]<collectlist[my_addr()][j-1]){
                            temp=collectlist[my_addr()][j];
                            collectlist[my_addr()][j]=collectlist[my_addr()][j-1];
                            collectlist[my_addr()][j-1]=temp;
                        }
                    }
                    printf("%d[",i);
                    for(int k=0;k<5;k++){
                        printf("%d ",collectlist[my_addr()][k]);
                    }
                    printf("]\n");
                }

				if (CODE_NUM == 2) {
					createCodepacket2(collectlist[my_addr()][0], collectlist[my_addr()][1],0);
					mystatus[my_addr()] = STA_FL;
					collectNum[my_addr()] = 0;
				} else if (CODE_NUM == 3) {
					createCodepacket3(collectlist[my_addr()][0], collectlist[my_addr()][1], collectlist[my_addr()][2],0);
					mystatus[my_addr()] = STA_FL;
					collectNum[my_addr()] = 0;
				} else if (CODE_NUM == 4) {
					createCodepacket4(collectlist[my_addr()][0], collectlist[my_addr()][1], collectlist[my_addr()][2],
									  collectlist[my_addr()][3],0);
					mystatus[my_addr()] = STA_FL;
					collectNum[my_addr()] = 0;
				} else if (CODE_NUM == 5) {
					createCodepacket5(collectlist[my_addr()][0], collectlist[my_addr()][1], collectlist[my_addr()][2],
									  collectlist[my_addr()][3], collectlist[my_addr()][4],0);
					mystatus[my_addr()] = STA_FL;
					collectNum[my_addr()] = 0;
				}
			}
		}
		else if(mystatus[my_addr()]==STA_HYBRID){
		    //ここから符号収集
            collectlist[my_addr()][collectNum[my_addr()]] = ph->pktnum_;
            collectNum[my_addr()]++;
            fprintf(mytraceFile, "hyb\t%f\tnode:%d\tfrom:%d\ttype:%d\tpktNo:%d \n", Scheduler::instance().clock(),
                    my_addr(), ph->addr(), ph->pkttype_, ph->pktnum_);
            if(collectNum[my_addr()]+1==CODE_NUM) {//あとひとつあつまれば送信できるとき
                mystatus[my_addr()] = STA_CODESENDREADY;
            }
            //ここまで
            //ノーマルを1つ送信
            int tempaddr = ph->addr();
            ph->addr() = my_addr();
            ph->pkttype_ = PKT_NORMAL;
            ch->next_hop() = IP_BROADCAST;
            ch->addr_type() = NS_AF_NONE;
            ch->direction() = hdr_cmn::DOWN;
            fprintf(mytraceFile, "hyb_f\t%f\tnode:%d\tfrom:%d\ttype:%d\tpktNo:%d\n", Scheduler::instance().clock(), my_addr(),tempaddr,ph->pkttype_ ,ph->pktnum_);

            //即時送信
            send(p,0);
            //遅延送信
            //Scheduler::instance().schedule(target_,p,0.01 * Random::uniform());
		}
		else{
		    fprintf(stdout,"Send Err\n");
            return;
		}
		return;
	}
}


int SBAgent::createCodepacket2(int pkt_1, int pkt_2, int encode_count){
    Packet* p = allocpkt();
    struct hdr_cmn* ch = HDR_CMN(p);
    struct hdr_ip* ih = HDR_IP(p);
    struct hdr_beacon * ph = HDR_BEACON(p);

    ph->addr() = my_addr();
    ph->seq_num() = seq_num_++;
    ph->pktnum_=codepktnum;
    ph->pkttype_=PKT_CODED;
    ch->ptype() = PT_SB;
    ph->codenum_=2;
    ph->codevc_=rand()%GALOIS+1;
    ph->pkt1_=pkt_1;
    ph->pkt2_=pkt_2;
    ph->pkt3_=-1;
    ph->pkt4_=-1;
    ph->pkt5_=-1;
    ph->hop_count_=1;
    encode_count++;
    ph->encode_count_=encode_count;

    ch->direction() = hdr_cmn::DOWN;
    ch->size() = IP_HDR_LEN;
    ch->error() = 0;    ch->next_hop() = IP_BROADCAST;
    ch->addr_type() = NS_AF_INET;

    ih->saddr() = my_addr();
    ih->daddr() = IP_BROADCAST;
    ih->sport() = RT_PORT;
    ih->dport() = RT_PORT;
    ih->ttl() = IP_DEF_TTL;

    fprintf(createcodeFile,"node:%d\t%f\tpktNo:%d\tvc:%d\t%d\t%d\t%d\t%d\t%d\n",my_addr(),Scheduler::instance().clock(),ph->pktnum_,ph->codevc_,ph->pkt1_,ph->pkt2_,ph->pkt3_,ph->pkt4_,ph->pkt5_);

	fprintf(mytraceFile, "sc\t%f\tnode:%d\tfrom:%d\ttype:%d\tpktNo:%d\tcv:%d\ttopo:NaN\tstatus:%d\tnei:NaN\t[ %d %d %d %d %d]\n",
			Scheduler::instance().clock(), my_addr(),ph->addr(),ph->pkttype_, ph->pktnum_,ph->codevc_,mystatus[my_addr()],ph->pkt1_,ph->pkt2_,ph->pkt3_,ph->pkt4_,ph->pkt5_);
    //即時送信
    send(p,0);
    //遅延送信
    //Scheduler::instance().schedule(target_,p,0.01 * Random::uniform());
	codepktnum++;
}
int SBAgent::createCodepacket3(int pkt_1, int pkt_2 , int pkt_3, int encode_count) {
	fprintf(createcodeFile,"%f node:%d :%d + %d + %d\n",Scheduler::instance().clock(),my_addr(),pkt_1,pkt_2,pkt_3);
	Packet* p = allocpkt();
	struct hdr_cmn* ch = HDR_CMN(p);
	struct hdr_ip* ih = HDR_IP(p);
	struct hdr_beacon * ph = HDR_BEACON(p);

	ph->addr() = my_addr();
	ph->seq_num() = seq_num_++;

	ph->pktnum_=codepktnum;
	ph->pkttype_=PKT_CODED;
	ch->ptype() = PT_SB;
	ph->codenum_=3;
	ph->codevc_=rand()%256+1;
	ph->pkt1_=pkt_1;
	ph->pkt2_=pkt_2;
	ph->pkt3_=pkt_3;
	ph->pkt4_=-1;
	ph->pkt5_=-1;
    ph->hop_count_=1;
    encode_count++;
    ph->encode_count_=encode_count;

    fprintf(mytraceFile,"%d ,%d\n",encode_count,ph->encode_count_);
    ch->direction() = hdr_cmn::DOWN;
	ch->size() = IP_HDR_LEN;
	ch->error() = 0;
	ch->next_hop() = IP_BROADCAST;
	ch->addr_type() = NS_AF_INET;

	ih->saddr() = my_addr();
	ih->daddr() = IP_BROADCAST;
	ih->sport() = RT_PORT;
	ih->dport() = RT_PORT;
	ih->ttl() = IP_DEF_TTL;
    fprintf(createcodeFile,"node:%d\t%f\tpktNo:%d\tvc:%d\t%d\t%d\t%d\t%d\t%d\n",my_addr(),Scheduler::instance().clock(),ph->pktnum_,ph->codevc_,ph->pkt1_,ph->pkt2_,ph->pkt3_,ph->pkt4_,ph->pkt5_);

    fprintf(mytraceFile, "sc\t%f\tnode:%d\tfrom:%d\ttype:C\tpktNo:%d\tcv:%d\tencode:%d\n", Scheduler::instance().clock(), my_addr(),my_addr(),ph->pktnum_,ph->codevc_,encode_count);
    //即時送信
    send(p,0);
    //遅延送信
    //Scheduler::instance().schedule(target_,p,0.01 * Random::uniform());
	codepktnum++;

}
int SBAgent::createCodepacket4(int pkt_1, int pkt_2 , int pkt_3, int pkt_4, int encode_count) {
	fprintf(stdout,"create4:%d + %d + %d + %d\n",pkt_1,pkt_2,pkt_3,pkt_4);
	Packet* p = allocpkt();
	struct hdr_cmn* ch = HDR_CMN(p);
	struct hdr_ip* ih = HDR_IP(p);
	struct hdr_beacon * ph = HDR_BEACON(p);

	ph->addr() = my_addr();
	ph->seq_num() = seq_num_++;

	ph->pktnum_=codepktnum;
	ph->pkttype_=PKT_CODED;
	ch->ptype() = PT_SB;
	ph->codenum_=4;
	ph->codevc_=rand()%256+1;

	ph->pkt1_=pkt_1;
	ph->pkt2_=pkt_2;
	ph->pkt3_=pkt_3;
	ph->pkt4_=pkt_4;
	ph->pkt5_=-1;
    ph->hop_count_=1;
    ph->encode_count_=encode_count++;


    ch->direction() = hdr_cmn::DOWN;
	ch->size() = IP_HDR_LEN;
	ch->error() = 0;
	ch->next_hop() = IP_BROADCAST;
	ch->addr_type() = NS_AF_INET;

	ih->saddr() = my_addr();
	ih->daddr() = IP_BROADCAST;
	ih->sport() = RT_PORT;
	ih->dport() = RT_PORT;
	ih->ttl() = IP_DEF_TTL;
    fprintf(createcodeFile,"node:%d\t%f\tpktNo:%d\tvc:%d\t%d\t%d\t%d\t%d\t%d\n",my_addr(),Scheduler::instance().clock(),ph->pktnum_,ph->codevc_,ph->pkt1_,ph->pkt2_,ph->pkt3_,ph->pkt4_,ph->pkt5_);

    fprintf(mytraceFile, "sc\t%f\tnode:%d\tfrom:%d\ttype:C\tpktNo:%d\tcv:%d\n", Scheduler::instance().clock(), my_addr(),my_addr(),ph->pktnum_,ph->codevc_);
    //即時送信
    send(p,0);
    //遅延送信
    //Scheduler::instance().schedule(target_,p,0.01 * Random::uniform());
}
int SBAgent::createCodepacket5(int pkt_1, int pkt_2 , int pkt_3, int pkt_4, int pkt_5, int encode_count) {
	fprintf(stdout,"create5\n");
	Packet* p = allocpkt();
	struct hdr_cmn* ch = HDR_CMN(p);
	struct hdr_ip* ih = HDR_IP(p);
	struct hdr_beacon * ph = HDR_BEACON(p);

	ph->addr() = my_addr();
	ph->seq_num() = seq_num_++;

	ph->pktnum_=codepktnum;
	ph->pkttype_=PKT_CODED;
	ch->ptype() = PT_SB;
	ph->codenum_=5;
	ph->codevc_=rand()%256+1;

	ph->pkt1_=pkt_1;
	ph->pkt2_=pkt_2;
	ph->pkt3_=pkt_3;
	ph->pkt4_=pkt_4;
	ph->pkt5_=pkt_5;
    ph->hop_count_=1;
    ph->encode_count_=encode_count++;


    ch->direction() = hdr_cmn::DOWN;
	ch->size() = IP_HDR_LEN;
	ch->error() = 0;
	ch->next_hop() = IP_BROADCAST;
	ch->addr_type() = NS_AF_INET;

	ih->saddr() = my_addr();
	ih->daddr() = IP_BROADCAST;
	ih->sport() = RT_PORT;
	ih->dport() = RT_PORT;
	ih->ttl() = IP_DEF_TTL;
    fprintf(createcodeFile,"node:%d\t%f\tpktNo:%d\tvc:%d\t%d\t%d\t%d\t%d\t%d\n",my_addr(),Scheduler::instance().clock(),ph->pktnum_,ph->codevc_,ph->pkt1_,ph->pkt2_,ph->pkt3_,ph->pkt4_,ph->pkt5_);

    fprintf(mytraceFile, "sc\t%f\tnode:%d\tfrom:%d\ttype:C\tpktNo:%d\tcv:%d\n", Scheduler::instance().clock(), my_addr(),my_addr(),ph->pktnum_,ph->codevc_);
    //即時送信
    send(p,0);
    //遅延送信
    //Scheduler::instance().schedule(target_,p,0.01 * Random::uniform());
}
