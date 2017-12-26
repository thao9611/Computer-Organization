#include <bits/stdc++.h>

using namespace std;

#define si pair<string,int>
#define ss pair<string,string>

int ndata; 		//size of data;
string data;	//binary string of initial value;
string ins; 	// the result binary
int nins;		//number of instruction
int address= 4194304; //initial address of instruction

map<string,int> dTable;		//address table for data
map<string,string> opTable; //opcode for instruction

set<si> jset;				
set<si> bset;

set <string> r;		
set <string> rshift;
set <string> i;
set <string> iofs;
set <string> j;
set <string> ibranch;

//String process for .data
//3B
void dataProcess(){
	string s;
	string c;
	int k;
	int address = 268435456;  //initial address of data
	
	while (true){
		cin >> s;
		if (s==".text") break;
		//handle variable type and value
		if (s[0]=='.'){   
			cin >> c;
			//convert value to decimal int;
			if (c[1]=='x') k=(int)strtol(c.c_str(), NULL, 0); else k=atoi(c.c_str());
			if (s==".word"){
				bitset<32> b(k);
				data = data + b.to_string();
				address+=4;
				ndata+=4;
			}
		//handle variable name
		}else{			  
			dTable.insert(si(s,address));
		}
	}
}

//Init set for group instruction and opcode map
void init(){
	r.insert("addu");r.insert("and"); r.insert("nor");r.insert("or");r.insert("sltu"); r.insert("subu");
	rshift.insert("sll");rshift.insert("srl");
	i.insert("addiu");i.insert("andi");i.insert("ori");i.insert("sltiu");
	iofs.insert("lw");iofs.insert("sw");
	j.insert("j"); j.insert("jal");
	ibranch.insert("beq");ibranch.insert("bne");
	
	opTable.insert(ss("addu","100001"));
	opTable.insert(ss("and","100100"));
	opTable.insert(ss("jr","001000"));
	opTable.insert(ss("nor","100111"));
	opTable.insert(ss("or","100101"));
	opTable.insert(ss("sltu","101011"));
	opTable.insert(ss("sll","000000"));
	opTable.insert(ss("srl","000010"));
	opTable.insert(ss("subu","100011"));
	opTable.insert(ss("addiu","001001"));
	opTable.insert(ss("andi","001100"));
	opTable.insert(ss("beq","000100"));
	opTable.insert(ss("bne","000101"));
	opTable.insert(ss("lui","001111"));
	opTable.insert(ss("lw","100011"));
	opTable.insert(ss("ori","001101"));
	opTable.insert(ss("sltiu","001011"));
	opTable.insert(ss("sw","101011"));
	opTable.insert(ss("j","000010"));
	opTable.insert(ss("jal","000011"));
}

//Convert immediate ins to binary
string iRegister(string op, int s, int t, int i){
	bitset<5> rs(s);
	bitset<5> rt(t);
	bitset<16> imm(i);

	nins+=4;
	address+=4;

	return op + rs.to_string() + rt.to_string()+ imm.to_string();
}

//Convert jum ins to binary
string jRegister(string op, int add){
	bitset<26> a(add);
	nins+=4;
	address+=4;
	return op + a.to_string();
}

//Convert register ins to binary
string rRegister( int s, int t, int d, int h, string op){
	bitset<5> rs(s);
	bitset<5> rt(t);
	bitset<5> rd(d);
	bitset<5> sh(h);
	
	nins+=4;
	address+=4;
	
	return "000000" + rs.to_string() + rt.to_string() + rd.to_string() + sh.to_string() + op;
}

//register process
void rProcess(string i){
	string s;
	string t;
	string d;
	cin >> d;
	cin >> s;
	cin >> t;
	int regs= atoi(s.substr(1,s.size()-1).c_str());
	int regt= atoi(t.substr(1,t.size()-1).c_str());
	int regd= atoi(d.substr(1,d.size()-1).c_str());
	ins += rRegister(regs, regt, regd, 0, opTable[i]);
}

//shift register process
void rshiftProcess(string i){
	string h;
	string rt;
	string rd;
	cin >> rd;
	cin >> rt;
	cin >> h;
	int t = atoi(rt.substr(1,rt.size()-1).c_str()); 
	int d = atoi(rd.substr(1,rd.size()-1).c_str());
	ins += rRegister(0,t,d,atoi(h.c_str()),opTable[i]);
}

//immediate process
void iProcess(string i){
	string rs;
	string rt;
	string imm;
	cin >> rt;
	cin >> rs;
	cin >> imm;
	
	int s = atoi(rs.substr(1,rs.size()-1).c_str()); 
	int t = atoi(rt.substr(1,rt.size()-1).c_str());
	int immint;
	if (imm[1]=='x') immint= (int)strtol(imm.c_str(), NULL, 0); else immint=atoi(imm.c_str());
	ins += iRegister(opTable[i],s, t, immint);
}

//offset immediate process
void iofsProcess(string i){
	string t;
	string offset;
	string s="";
	string off="";
	cin >> t;
	cin >> offset;
	int regt= atoi(t.substr(1,t.size()-1).c_str());
	
	int k=0;
	while (offset[k]!='(') off+=offset[k++];
	k++;
	while (offset[k]!=')') s+=offset[k++];
	
	int regs= atoi(s.substr(1,s.size()-1).c_str());
	int imm = atoi(off.c_str());
	ins+=iRegister(opTable[i], regs, regt, imm);
	
}

//branch immediate process
void ibranchProcess(string i){
	string t;
	string s;
	string lab;
	cin >> s;
	cin >> t;
	cin >> lab;
	int regt = atoi(t.substr(1,t.size()-1).c_str());
	int regs = atoi(s.substr(1,s.size()-1).c_str());
	ins += iRegister(opTable[i],regs,regt,0);
	bset.insert(si(lab,nins));
}

//jump register process
void jProcess(string i){
	string t;
	cin >> t;
	jset.insert(si(t, nins));
	ins+= jRegister(opTable[i],0);
}

//lui immediate process
void luiProcess(){
	string t;
	string imm;
	cin >> t;
	cin >> imm;
	int regt = atoi(t.substr(1,t.size()-1).c_str());
	int immint;
	if (imm[1]=='x') immint= (int)strtol(imm.c_str(), NULL, 0); else immint=atoi(imm.c_str());
	ins += iRegister(opTable["lui"],0,regt,immint);
}

void jrProcess(){
	string rd;
	cin >> rd;
	int d = atoi(rd.substr(1,rd.size()-1).c_str()); 
	ins += rRegister(d,0,0,0, opTable["jr"]);
}

//la process
void laProcess(){
	string s;
	cin >> s;
	int regs = atoi(s.substr(1,s.size()-1).c_str());
	
	string lab;
	cin >> lab;
	lab+=':';
	
	int adr = dTable[lab];
	bitset<32> b(adr);
	bitset<16> upper(b.to_string().substr(0,16));
	bitset<16> lower(b.to_string().substr(16,32));
	
	int u =  upper.to_ulong();
	int l =  lower.to_ulong();
	
	ins += iRegister(opTable["lui"],0,regs,u);
	if (l!=0){
		ins += iRegister(opTable["ori"],regs,regs,l);
	}
}

//String process for .text
//3B
void textProcess(){
	string s;
	string c;
	
	cin >> s;
	dTable.insert(si(s,address));
	while (true){
		cin >> s;
		if (cin.eof()) break;
		//Handle label
		if (s[s.size()-1]==':') {
			dTable.insert(si(s,address));
		//Handle instruction
		}else{
			//cout << s;
			if ( r.find(s) != r.end()) rProcess(s);
			if ( rshift.find(s) != rshift.end()) rshiftProcess(s);
			if ( i.find(s) != i.end()) iProcess(s);
			if ( iofs.find(s) != iofs.end()) iofsProcess(s);
			if ( j.find(s) != j.end()) jProcess(s);
			if ( ibranch.find(s) != ibranch.end()) ibranchProcess(s);
			if ( s == "lui") luiProcess();
			if ( s == "jr") jrProcess();
			if (s == "la") laProcess();
		}
	}
}

//Branch insert address
void bAddress(){
	for (set<si>::iterator it=bset.begin(); it!=bset.end(); it++){
		string lab= (it->first) + ':';
		int n = (it->second)/4;
		int offset = ((dTable[lab]-4194304)/4)-n;
		bitset<16> boffset(offset);
		string sos=boffset.to_string();
		for (int k=0; k<16; k++) ins[(n-1)*32+16+k]=sos[k];
	}
}

void jAddress(){
	// change the address of jump destination 
	// dTable: table of ( the label and address), each is 4 in between
	//jset: the set of the ith instruction and the label 
	// ins: the result which needs to be adjusted
	for (set<si>::iterator it = jset.begin(); it != jset.end(); ++it){
		int i = it->second; //nins
		string label = (it->first)+':';
		int addr = dTable[label];
		bitset<26> binadd(addr/4);
		//cout << binadd.to_string()<< endl;
		string sos=binadd.to_string();
		for (int k=0; k<26; k++) ins[(i)*8+6+k]=sos[k];
		
	}
}
//Seperate .data part and .text part
//3B
void Process(){
	string s;
	while (s!=".data") cin >>s;	
	init();
	dataProcess();
	textProcess();
	
	string res="";
	
	bitset<32> bdata(ndata);
	bitset<32> bins(nins);
	
	bAddress();
	jAddress();
	
	res = res  + bins.to_string() + bdata.to_string() + ins + data;
	cout << res;
/*	for (int i=0; i<res.size(); i++){
		cout << res[i];
		if (i%32==31) cout << endl;
	}*/
}

int main(int argc, char* argv[]){
	string fi = argv[1];
	string fo = fi;
	fo[fo.size()-1] = 'o';
	freopen(fi.c_str(),"r",stdin);
	freopen(fo.c_str(),"w",stdout);
	Process();
	return 0;
}




