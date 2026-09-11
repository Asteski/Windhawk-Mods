#include <windows.h>
#include <dbghelp.h>
#include <stdio.h>
BOOL CALLBACK Enum(PSYMBOL_INFO s, ULONG, PVOID) { printf("%llx %s\n",s->Address,s->Name); return TRUE; }
int main(int argc,char**argv) {
 SymSetOptions(SYMOPT_UNDNAME|SYMOPT_DEFERRED_LOADS|SYMOPT_DEBUG);
 HANDLE p=GetCurrentProcess(); if(!SymInitialize(p,"srv*C:\\Users\\Adams\\Code\\windhawk-mods\\.codex-build\\symbols*https://msdl.microsoft.com/download/symbols",FALSE)) return 1;
 DWORD64 b=SymLoadModuleEx(p,nullptr,argv[1],nullptr,0x180000000,0,nullptr,0); printf("base %llx error %lu\n",b,GetLastError());
 SymEnumSymbols(p,b,argc>2?argv[2]:"*Taskbar*",Enum,nullptr);
}
