#include <stdio.h>
#include "BPatch.h"
#include "BPatch_addressSpace.h"
#include "BPatch_process.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_point.h"
#include "BPatch_function.h"

using namespace std;
using namespace Dyninst;

string progName;

// Create an instance of class BPatch
BPatch bpatch;

BPatch_addressSpace* startInstrumenting(const char* name)
{
    BPatch_addressSpace* handle = NULL;

    // Open the binary file and all dependencies
    handle = bpatch.openBinary(name, true); // TODO : true if libc functions are nedded (slow), otherwise false
    if (!handle) 
    { 
        fprintf(stderr, "openBinary failed\n"); 
    }

    // load the shared library that myfunc resides in
    if (!handle->loadLibrary("libmyfunc.so")) 
    {
        fprintf(stderr, "Failed to open instrumentation library\n");
        exit(EXIT_FAILURE);
    }

    return handle;
}

BPatch_function *findFunc(BPatch_image *image, const char *name) 
{
    std::vector<BPatch_function*> funcs;

    image->findFunction(name, funcs);
    if (funcs.empty()) 
    {
        printf("Could not find any function named %s\n",name);
        exit(EXIT_FAILURE);
    }
    if (funcs.size() > 1) 
    {
        printf("Found more than one function named %s\n",name);
        exit(EXIT_FAILURE);
    }
    return funcs[0];
}

bool is_linker_added_func(string func_name)
{
    // TODO : func_name[0] == '_' might be wrong
    if(func_name[0] == '_' || func_name == "deregister_tm_clones" || func_name == "register_tm_clones" || func_name == "frame_dummy")
    {
        return true;
    }

    return false;
}

void advanced_instrumentation(BPatch_addressSpace* app)
{
    std::vector<BPatch_function*> *all_funcs = NULL;
    BPatch_image *image = app->getImage();
    

    std::vector<BPatch_module *> *modules; 
    modules = image->getModules();

    // get all functions from the "progName" module
    for(auto mit : *modules)
    {
        BPatch_module* module = mit;
        char name_str[30];
        module->getName(name_str,sizeof(name_str));

        if(string(name_str) == string(progName))
        {
            all_funcs = module->getProcedures();
            break;
        }
    }

    // cannot find mutatee
    if(!all_funcs)
    {
        fprintf(stderr, "failed to find mutatee\n");
        exit(EXIT_FAILURE);
    }
    
    for(auto fit : *all_funcs)
    {
        BPatch_function *func = fit;

        // skip linker added functions (TODO)
        string func_name = func->getName();
        if(is_linker_added_func(func->getName()))
        {
            printf("skip function %s\n",func->getName().c_str());
            continue;
        }

        /* Find all pointcuts where a subroutine is invoked. */
        std::vector<BPatch_point *> *lib_calls = func->findPoint(BPatch_subroutine);
        /* Loop over these pointcuts. */
        for (std::vector<BPatch_point *>::iterator ip = lib_calls->begin();
                                                    ip != lib_calls->end();
                                                    ++ip) 
        {
            BPatch_point *point = *ip;

            /* We are only interested in indirect call sites. */
            if (point->isDynamic()) 
            {   
                BPatch_function *my_func = findFunc(app->getImage(), "myfunc");
                
                /* Now we can setup the argument vector... */
                std::vector<BPatch_snippet*> args;
                BPatch_snippet* oae = new BPatch_originalAddressExpr();
                args.push_back(oae);
                BPatch_snippet* dte = new BPatch_dynamicTargetExpr();
                args.push_back(dte);

                /* ...and construct and insert the snippet. */
                BPatch_funcCallExpr myfunc_call_expr( *my_func, args);
                if (!app->insertSnippet(myfunc_call_expr, *point, BPatch_callBefore)) 
                {
                    fprintf(stderr, "ERROR: failed to insert snippet at callback pointcut\n");
                    exit(EXIT_FAILURE);
                }
                else
                {
                    printf("insert a snippet at icall (addr : %p)\n", point->getAddress());
                }
            }
            // also instrument direct calls.
            else
            {
                BPatch_function *printf_func = findFunc(app->getImage(), "printf");

                /* Now we can setup the argument vector... */
                std::vector<BPatch_snippet*> args;
                BPatch_snippet* fmt = new BPatch_constExpr("dcall addr is : %p\n");
                args.push_back(fmt);
                BPatch_snippet* oae = new BPatch_originalAddressExpr();
                args.push_back(oae);

                /* ...and construct and insert the snippet. */
                BPatch_funcCallExpr myfunc_call_expr( *printf_func, args);
                if (!app->insertSnippet(myfunc_call_expr, *point, BPatch_callBefore)) 
                {
                    fprintf(stderr, "ERROR: failed to insert snippet at callback pointcut\n");
                    exit(EXIT_FAILURE);
                }
                else
                {
                    printf("insert a snippet at dcall (addr : %p)\n", point->getAddress());
                }
            }
        } // loop over pointcuts
    }
}

void finishInstrumenting(BPatch_addressSpace* app, const char* newName)
{
    BPatch_binaryEdit* appBin = dynamic_cast<BPatch_binaryEdit*>(app);
    if (appBin) 
    {
        if (!appBin->writeFile(newName)) 
        {
            fprintf(stderr, "writeFile failed\n");
        }
    }
}

int main(int argc, char **argv) 
{

    // Set up information about the program to be instrumented
    progName = "mutatee";   // TODO : progName is hard-coded here

    // rewrite a binary
    BPatch_addressSpace* app = startInstrumenting(progName.c_str());
    if (!app) 
    {
        fprintf(stderr, "startInstrumenting failed\n");
        exit(1);
    }

    advanced_instrumentation(app);

    // Finish instrumentation
    string progName2 = progName + "-rewritten";
    finishInstrumenting(app, progName2.c_str());

    printf("instrumentation finished\n");

}