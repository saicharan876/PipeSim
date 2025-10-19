#include<bits/stdc++.h>
using namespace std;
#define endl "\n"
#define BYTE int

BYTE R[16];                                   //Register array
BYTE dataCache[256];                          //datacache array
BYTE instructionCache[256];                   //instruction cache array
int PC=0;                                     // Program counter
int IR=0;                                     // Instruction register

// Parameters we are measuring
int clock_cycles=0;
int datastalls=0;
int controlstalls=0;
int arithmeticInstructions=0;
int logicalInstructions=0;
int dataInstructions=0;
int controlInstructions=0;
int haltInstructions=0;

bool Rutd[16];                               // stores if Register is up to date(for data dependency hazards)

BYTE A,B;                                    // Input to ALU
BYTE ALUOutput;                              // Alu outpue
BYTE LMD;                                    // LMD register


string WB_instruction="Stall";              // stall, aluoutput, lmd
int WB_outputreg;                           // the register we wb to
int lastupdated;                            // for data dependency
int WB_ALUOutput;                           //the aluoutput for the instruction
void WB()                                   // WB block of the pipelined processor
{
    lastupdated=-1;
    if(WB_instruction=="Stall")return;
    if(WB_instruction=="ALUOutput")
    {
        R[WB_outputreg]=WB_ALUOutput;
    }
    if(WB_instruction=="Load")
    {
        R[WB_outputreg]=LMD;
    }
    Rutd[WB_outputreg]=true;
    lastupdated=WB_outputreg;
}

string MEM_instruction="Stall";             //stall, read , write
string MEM_WB_instruction="Stall";          // wb instruction in the next cycle
int MEM_WB_outputreg;                       //wb output register for the next cycle
void MEM()                                  // MEM block in the pipelined processor
{
    WB_instruction=MEM_WB_instruction;
    WB_outputreg=MEM_WB_outputreg;
    WB_ALUOutput=ALUOutput;
    if(MEM_instruction=="Stall")
    {
        return;
    }
    if(MEM_instruction=="Read")
    {
        LMD=dataCache[ALUOutput];
    }
    if(MEM_instruction=="Write")
    {
        dataCache[ALUOutput]=R[MEM_WB_outputreg];
    }
    if(MEM_instruction=="Branch")
    {
        PC=ALUOutput;
    }
}

string ALU_instruction="Stall";             //stall, memoryreference , branch , or operation
string ALU_MEM_instruction="Stall";         // instruction for mem in the next clock cyle
string ALU_WB_instruction="Stall";          // instruction for wb for this instruction
int ALU_WB_outputreg;                       // output register for the wb for this instruction
int ALU_opcode;                             // opcode of this instruction
int ALU_old_pc;                             //used for branch instruction
void EX()                                   // Execute/Effective address block of the pipelined process (ALU)
{
    MEM_instruction=ALU_MEM_instruction;
    MEM_WB_instruction=ALU_WB_instruction;
    MEM_WB_outputreg=ALU_WB_outputreg;
    if(ALU_instruction=="Stall")
    {
        return;
    }
    if(ALU_instruction=="MemoryReference")
    {
        dataInstructions++;
        ALUOutput=A+B;
    }
    if(ALU_instruction=="Branch")
    {
        controlInstructions++;
        ALUOutput=ALU_old_pc+(B*2);
        if(ALU_opcode==11)
        {
            if(A)
            {
                ALUOutput=ALU_old_pc;
            }
        }
    }
    if (ALU_opcode == 0)
    {
        ALUOutput = A + B;
        arithmeticInstructions++;
    }
    if (ALU_opcode == 1)
    {
        ALUOutput = A - B;
        arithmeticInstructions++;
    }
    if (ALU_opcode == 2)
    {
        ALUOutput = A*B;
        arithmeticInstructions++;
    }
    if (ALU_opcode == 3)
    {
        ALUOutput = A + 1;
        arithmeticInstructions++;
    }
    if (ALU_opcode == 4)
    {
        ALUOutput = A & B;
        logicalInstructions++;
    }
    if (ALU_opcode == 5)
    {
        ALUOutput = A | B;
        logicalInstructions++;
    }
    if (ALU_opcode == 6)
    {
        ALUOutput = ~A;
        logicalInstructions++;
    }
    if (ALU_opcode == 7)
    {
        ALUOutput = A ^ B;
        logicalInstructions++;
    }
    ALUOutput%=256;
}

int bits(int IR,int l,int r)        //returns the value form bits l to r
{
    int val=0;
    for(int i=l;i<r;i++)
    {
        if(IR&(1<<i))val+=(1<<i);
    }
    val>>=l;
    return val;
}

int stalls=0;                       // Number of stalls IF has to do
bool ID_active=0;                   // tells if we have to do ID in this clock cycle
bool halt=false;                    //if halt command is encountered
void ID()                           // Instruction decode block of the pipelined processor
{
    if(!ID_active || halt)
    {
        ALU_instruction="Stall";
        ALU_MEM_instruction="Stall";
        ALU_WB_instruction="Stall";
        return;
    }
    
    int op=(IR>>12);

    if(op==15)      //halt
    {
        haltInstructions++;
        halt=true;
        ALU_instruction="";
        ALU_MEM_instruction="";
        ALU_WB_instruction="";
        ALU_opcode=op;
        return;
    }

    if(op==10)    //jmp
    {
        int l1=0;
        for(int i=4;i<12;i++)
        {
            if(IR&(1<<i))l1+=1<<i;
        }
        l1=l1>>4;
        B=l1;
        ALU_instruction="Branch";
        ALU_MEM_instruction="Branch";
        ALU_WB_instruction="Stall";
        ALU_opcode=10;
        ALU_old_pc=PC;
        stalls+=2;
        controlstalls+=2;
    }

    if(op==11)    //beqz
    {
        int l1=0;
        for(int i=0;i<8;i++)
        {
            if(IR&(1<<i))l1+=1<<i;
        }
        B=l1;
        int index=0;
        for(int i=8;i<12;i++)
        {
            if(IR&(1<<i))index+=1<<i;
        }
        index=index>>8;
        if(!Rutd[index] || lastupdated==index)
        {
            stalls++;
            datastalls++;
            ALU_instruction="Stall";
            ALU_MEM_instruction="Stall";
            ALU_WB_instruction="Stall";
            return;
        }
        A=R[index];
        ALU_instruction="Branch";
        ALU_MEM_instruction="Branch";
        ALU_WB_instruction="Stall";
        ALU_opcode=11;
        ALU_old_pc=PC;
        stalls+=2;
        controlstalls+=2;
    }

    if(op==9)               //store
    {
        int X=0;
        for(int i=0;i<4;i++)
        {
            if(IR&(1<<i))X+=1<<i;
        }
        B=X;
        int index=0;        //r1 reg
        for(int i=8;i<12;i++)
        {
            if(IR&(1<<i))index+=1<<i;
        }
        index=index>>8;
        int r2=0;
        for(int i=4;i<8;i++)
        {
            if(IR&(1<<i))r2+=1<<i;
        }
        r2=r2>>4;
        if(!Rutd[r2] || lastupdated==r2 )
        {
            stalls++;
            datastalls++;
            ALU_instruction="Stall";
            ALU_MEM_instruction="Stall";
            ALU_WB_instruction="Stall";
            return;
        }
        A=R[r2];
        ALU_instruction="MemoryReference";
        ALU_MEM_instruction="Write";
        ALU_WB_instruction="Stall";
        ALU_WB_outputreg=index;
        ALU_opcode=9;
        ALU_old_pc=PC;
    }

    if(op==8)       // Load
    {
        int l1=0;
        for(int i=0;i<4;i++)
        {
            if(IR&(1<<i))l1+=1<<i;
        }
        B=l1;
        int index=0;
        for(int i=4;i<8;i++)
        {
            if(IR&(1<<i))index+=1<<i;
        }
        index>>=4;
        if(!Rutd[index] || lastupdated==index)
        {
            stalls++;
            datastalls++;
            ALU_instruction="Stall";
            ALU_MEM_instruction="Stall";
            ALU_WB_instruction="Stall";
            return;
        }
        A=R[index];
        ALU_instruction="MemoryReference";
        ALU_MEM_instruction="Read";
        ALU_WB_instruction="Load";
        ALU_WB_outputreg=bits(IR,8,12);
        ALU_opcode=8;
        ALU_old_pc=PC;
        Rutd[bits(IR,8,12)]=false;
    }

    if(op==0 || op==1 || op==2 || op==4 || op==5 || op==7)      // Add,sub,mul,and,or,xor
    {
        if(!Rutd[bits(IR,4,8)] || !Rutd[bits(IR,0,4)] || lastupdated==bits(IR,4,8) || lastupdated==bits(IR,0,4))
        {
            stalls++;
            datastalls++;
            ALU_instruction="Stall";
            ALU_MEM_instruction="Stall";
            ALU_WB_instruction="Stall";
            return;
        }
        A=R[bits(IR,4,8)];
        B=R[bits(IR,0,4)];
        ALU_instruction="";
        ALU_MEM_instruction="";
        ALU_WB_instruction="ALUOutput";
        ALU_WB_outputreg=bits(IR,8,12);
        ALU_opcode=op;
        ALU_old_pc=PC;
        Rutd[bits(IR,8,12)]=false;
    }
    
    if(op==3)       // inc
    {
        if(!Rutd[bits(IR,8,12)] || lastupdated==bits(IR,8,12))
        {
            stalls++;
            datastalls++;
            ALU_instruction="Stall";
            ALU_MEM_instruction="Stall";
            ALU_WB_instruction="Stall";
            return;
        }
        A=R[bits(IR,8,12)];
        ALU_instruction="";
        ALU_MEM_instruction="";
        ALU_WB_instruction="ALUOutput";
        ALU_WB_outputreg=bits(IR,8,12);
        ALU_opcode=op;
        ALU_old_pc=PC;
        Rutd[bits(IR,8,12)]=false;
    }

    if(op==6)       //not
    {
        if(!Rutd[bits(IR,4,8)] || lastupdated==bits(IR,4,8))
        {
            stalls++;
            datastalls++;
            ALU_instruction="Stall";
            ALU_MEM_instruction="Stall";
            ALU_WB_instruction="Stall";
            return;
        }
        A=R[bits(IR,4,8)];
        ALU_instruction="";
        ALU_MEM_instruction="";
        ALU_WB_instruction="ALUOutput";
        ALU_WB_outputreg=bits(IR,8,12);
        ALU_opcode=op;
        ALU_old_pc=PC;
        Rutd[bits(IR,8,12)]=false;
    }

    ID_active=false;
}

void IF()                       // Istruction fetch block of the pipelined processor
{
    if(halt)return;
    if(stalls )
    {
        stalls--;
        return;
    }
    IR=instructionCache[PC];
    IR=(IR<<8)+instructionCache[PC+1];
    PC+=2;
    ID_active=true;
}

int hextodec(string s)          // converts string s in hex to integer
{
    int ret = 0;
    if (s[0] > '9') ret += s[0] - 'a' + 10;
    else ret += s[0] - '0';
    ret *= 16;
    if (s[1] > '9') ret += s[1] - 'a' + 10;
    else ret += s[1] - '0';
    return ret;
}

string dectohex(BYTE val)       // converts int val to hex string
{
    int ret = (int)val;
    ret += 256;
    ret %= 256;
    int f = ret / 16;
    string s;
    if (f > 9) f = 'a' + f - 10;
    else f = f + '0';
    s.push_back(char(f));
    f = ret % 16;
    if (f > 9) f = 'a' + f - 10;
    else f = f + '0';
    s.push_back(char(f));
    return s;
}
void init()                     // initializing all the arrrays (reg,datacache,instruction cache)
{
    ifstream ICache("ICache.txt");
    ifstream DCache("DCache.txt");
    ifstream RF("RF.txt");
    
    string val;
    for (int i = 0; i < 256; i++){
        getline(ICache, val);
        instructionCache[i] = hextodec(val);
    }
    for (int i = 0; i < 256; i++){
        getline(DCache, val);
        dataCache[i] = hextodec(val);
    }
    for (int i = 0; i < 16; i++){
        getline(RF, val);
        R[i] = hextodec(val);
    }

    ICache.close();
    DCache.close();
    RF.close();

    for(auto &val:Rutd)val=1;
}

void finalout()                     // Outputs the specs to Output.txt file and the updated data cache to ODCache.txt
{
    ofstream OutD("ODCache.txt");
    for (int i = 0; i < 256; i++) OutD << dectohex(dataCache[i]) << endl;

    int totalInstructions=arithmeticInstructions+logicalInstructions+dataInstructions+controlInstructions+haltInstructions;

    ofstream outFile;
    outFile.open("Output.txt");

    outFile << "Total number of instructions executed: " <<totalInstructions<<endl;
    outFile << "Number of instructions in each class" <<endl;
    outFile << "Arithmetic instructions              : " <<arithmeticInstructions<<endl;
    outFile << "Logical instructions                 : " <<logicalInstructions<<endl;
    outFile << "Data instructions                    : " <<dataInstructions<<endl;
    outFile << "Control instructions                 : " <<controlInstructions<<endl;
    outFile << "Halt instructions                    : " <<haltInstructions<<endl;
    outFile << "Cycles Per Instruction               : " <<((double) clock_cycles / totalInstructions) <<endl;
    outFile << "Total number of stalls               : " <<datastalls+controlstalls <<endl;
    outFile << "Data stalls (RAW)                    : " <<datastalls <<endl;
    outFile << "Control stalls                       : " <<controlstalls <<endl;

    outFile.close();
    
}
int32_t main()
{
    init();                         // initialize values

    while(!(halt && ALU_instruction=="Stall" && MEM_instruction=="Stall" && WB_instruction=="Stall"))   //till we finish halting we run the processor
    {
        clock_cycles++;
        WB();
        MEM();
        EX();
        ID();
        IF();
    }

    finalout();                     //outputing and the details

	return 0;
}
