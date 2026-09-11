#include <cstdint>
#include <initializer_list>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
int main(int argc,char**argv) {
 setvbuf(stdout,nullptr,_IONBF,0); puts("start"); SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX); SetDllDirectoryW(L"C:\\Program Files\\Windhawk\\Compiler\\bin"); HMODULE llvm=LoadLibraryW(L"C:\\Program Files\\Windhawk\\Compiler\\bin\\libLLVM-20.dll");
 if(!llvm) {printf("load failed %lu\n",GetLastError());return 1;} for(auto name:{"LLVMInitializeX86TargetInfo","LLVMInitializeX86Target","LLVMInitializeX86TargetMC","LLVMInitializeX86Disassembler"}) { auto fn=GetProcAddress(llvm,name); printf("%s %p\n",name,fn); if(!fn)return 3; ((void(*)())fn)(); }
 auto create=(void*(*)(const char*,void*,int,void*,void*))GetProcAddress(llvm,"LLVMCreateDisasm");
 auto dis=(size_t(*)(void*,unsigned char*,uint64_t,uint64_t,char*,size_t))GetProcAddress(llvm,"LLVMDisasmInstruction");
 printf("create %p dis %p\n",create,dis); auto ctx=create("x86_64-pc-windows",nullptr,0,nullptr,nullptr);
 FILE* f=fopen(argv[1],"rb"); if(!f)return 2; fseek(f,0,SEEK_END); long size=ftell(f); rewind(f); auto raw=new unsigned char[size]; fread(raw,1,size,f); fclose(f); auto nt=(IMAGE_NT_HEADERS64*)(raw+((IMAGE_DOS_HEADER*)raw)->e_lfanew); auto mod=new unsigned char[nt->OptionalHeader.SizeOfImage]{}; auto sec=IMAGE_FIRST_SECTION(nt); for(unsigned j=0;j<nt->FileHeader.NumberOfSections;j++) memcpy(mod+sec[j].VirtualAddress,raw+sec[j].PointerToRawData,sec[j].SizeOfRawData); auto rva=strtoull(argv[2],nullptr,16); auto count=strtoul(argv[3],nullptr,16);
 auto bytes=(unsigned char*)mod+rva;
 for(size_t i=0;i<count;) {char out[1024]{}; auto n=dis(ctx,bytes+i,count-i,0x180000000+rva+i,out,sizeof(out)); if(!n)n=1; printf("%llx %s\n",0x180000000+rva+i,out); i+=n;}
}




