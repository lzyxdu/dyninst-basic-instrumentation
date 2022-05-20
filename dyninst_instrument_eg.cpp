#include <stdio.h>

#include "BPatch.h"
#include "BPatch_addressSpace.h"
#include "BPatch_process.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_point.h"
#include "BPatch_function.h"
#include "BPatch_basicBlock.h"
#include "BPatch_edge.h"
#include "BPatch_flowGraph.h"

#include "Instruction.h"
#include "Operand.h"
#include "Expression.h"
#include "Visitor.h"
#include "Register.h"
#include "BinaryFunction.h"
#include "Immediate.h"
#include "Dereference.h"
// #include "Parsing.h" 
#include "Edge.h"
#include "Symtab.h" 

using namespace std;
using namespace Dyninst;

// Create an instance of class BPatch
BPatch bpatch;

// Different ways to perform instrumentation
typedef enum {
    create,
    attach,
    open
} accessType_t; 

// Attach, create, or open a file for rewriting
BPatch_addressSpace* startInstrumenting(accessType_t accessType,
        const char* name,
        int pid,
        const char* argv[]) {
    BPatch_addressSpace* handle = NULL;

    switch(accessType) {
        case create:
            handle = bpatch.processCreate(name, argv);
            if (!handle) { fprintf(stderr, "processCreate failed\n"); }
            break;
        case attach:
            handle = bpatch.processAttach(name, pid);
            if (!handle) { fprintf(stderr, "processAttach failed\n"); }
            break;
        case open:
            // Open the binary file and all dependencies
            handle = bpatch.openBinary(name, true);
            if (!handle) { fprintf(stderr, "openBinary failed\n"); }
            break;
    }

    return handle;
}

//lzy add
void insert_before_CALL(BPatch_addressSpace* app) {
    BPatch_image* appImage = app->getImage();
    
    
    std::vector<BPatch_function *> funcs;
    BPatch_function *func;
    std::set<BPatch_basicBlock *> blks;
    BPatch_basicBlock *blk;
    BPatch_flowGraph *cfg;
    
    appImage->findFunction("main", funcs);
    func = funcs[0];
    cfg = func->getCFG();
    cfg->getAllBasicBlocks(blks);

    for(auto *blk : blks)
    {
        
        std::vector<InstructionAPI::Instruction::Ptr> insns;
        blk->getInstructions(insns);
        auto instr = insns.rbegin();
        if(  (*instr)->format().find("call") != string::npos)
        {
            //create printf
            std::vector<BPatch_snippet*> printfArgs;
            BPatch_snippet* fmt = new BPatch_constExpr("inserted before call\n");
            printfArgs.push_back(fmt);
            std::vector<BPatch_function*> printfFuncs;
            appImage->findFunction("printf", printfFuncs);
            if (printfFuncs.size() == 0) {
                fprintf(stderr, "Could not find printf\n");
                exit(-1);
            }

            //insert printf at blk exit
            BPatch_funcCallExpr printfCall(*(printfFuncs[0]), printfArgs);
            if ( !app->insertSnippet(printfCall,*( blk->findExitPoint() ) ) ) {
                fprintf(stderr, "insertSnippet before call failed\n");
                exit(-1);
            }
        }
    }

}

void insert_at_FUNC_ENTRY(BPatch_addressSpace* app)
{
    BPatch_image* appImage = app->getImage();
    
    
    std::vector<BPatch_function *> funcs;
    BPatch_function *func;
    std::set<BPatch_basicBlock *> blks;
    BPatch_basicBlock *blk;
    BPatch_flowGraph *cfg;
    
    appImage->findFunction("do_nothing", funcs);
    if(funcs.size() != 1)
    {
        cout<<"can't find function:do_nothing"<<endl;
        exit(-1);
    }
    func = funcs[0];
    cfg = func->getCFG();

    std::vector<BPatch_point *> points;
    func->getEntryPoints(points);

    std::vector<BPatch_snippet*> printfArgs;
    BPatch_snippet* fmt = new BPatch_constExpr("inserted at func entry\n");
    printfArgs.push_back(fmt);
    std::vector<BPatch_function*> printfFuncs;
    appImage->findFunction("printf", printfFuncs);
    if (printfFuncs.size() == 0) {
        fprintf(stderr, "Could not find printf\n");
        exit(-1);
    }
    BPatch_funcCallExpr printfCall(*(printfFuncs[0]), printfArgs);
    if ( !app->insertSnippet(printfCall,points) ) {
        fprintf(stderr, "insertSnippet before call failed\n");
        exit(-1);
    }
}

void finishInstrumenting(BPatch_addressSpace* app, const char* newName)
{
    BPatch_process* appProc = dynamic_cast<BPatch_process*>(app);
    BPatch_binaryEdit* appBin = dynamic_cast<BPatch_binaryEdit*>(app);

    if (appProc) {
        if (!appProc->continueExecution()) {
            fprintf(stderr, "continueExecution failed\n");
        }
        while (!appProc->isTerminated()) {
            bpatch.waitForStatusChange();
        }
    } else if (appBin) {
        if (!appBin->writeFile(newName)) {
            fprintf(stderr, "writeFile failed\n");
        }
    }
}

int main() {
    // Set up information about the program to be instrumented
    const char* progName = "do_nothing";
    int progPID = 42;
    const char* progArgv[] = {"do_nothing", "-h", NULL};
    accessType_t mode = create;

    // Create/attach/open a binary
    BPatch_addressSpace* app = 
        startInstrumenting(mode, progName, progPID, progArgv);
    if (!app) {
        fprintf(stderr, "startInstrumenting failed\n");
        exit(1);
    }
    insert_before_CALL(app);
    insert_at_FUNC_ENTRY(app);

    // Finish instrumentation
    const char* progName2 = "donothing-rewritten";
    finishInstrumenting(app, progName2);
}
