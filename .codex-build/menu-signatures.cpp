#include <windows.h>
#include <dbghelp.h>
#include <stdio.h>
BOOL CALLBACK Enum(PSYMBOL_INFO s, ULONG, PVOID) { char name[8192]{}; UnDecorateSymbolName(s->Name,name,sizeof(name),0); printf("%llx %s\n",s->Address,name); return TRUE; }
int main(int argc,char**argv) {
 SymSetOptions(SYMOPT_DEFERRED_LOADS);
 HANDLE p=GetCurrentProcess(); if(!SymInitialize(p,"C:\\Users\\Adams\\Code\\windhawk-mods\\.codex-build\\symbols",FALSE)) return 1;
 DWORD64 b=SymLoadModuleEx(p,nullptr,argv[1],nullptr,0x180000000,0,nullptr,0);
 SymEnumSymbols(p,b,argc>2?argv[2]:"*",Enum,nullptr);
}
