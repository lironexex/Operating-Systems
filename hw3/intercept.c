#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/kallsyms.h>
#include <linux/moduleparam.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Liron Haber - 205732852, Arnon Kidron - 205893985");


typedef unsigned long linuxPtr;
typedef struct task_struct task_t;

void** sys_call_table = NULL;
char* program_name;
module_param(program_name, charp, S_IRUGO);

/*
 * Input: 2 strings with less than 16 characters
 * Output: whether they're exactly the same
 */
int is_name_exactly_same(char* str1, char* str2){
    return strncmp(str1, str2, 16) == 0;
}

/*
 * Input: address a page table entry
 * Output: a pointer to it
 */
pte_t* find_pte(linuxPtr addr){
    unsigned int level;
    return lookup_address(addr, &level);
}

/*
turns on the R/W flag for addr.
*/
void allow_rw(linuxPtr addr){
    //find PTE
    pte_t* pPte = find_pte(addr);

    //change permissions
    if(pPte->pte &~ _PAGE_RW)
        pPte->pte |= _PAGE_RW;
}

/*
turns off the R/W flag for addr.
*/
void disallow_rw(linuxPtr addr) {
    //find PTE
    pte_t* pPte = find_pte(addr);

    //change permissions
    pPte->pte = pPte->pte &~ _PAGE_RW;
}



asmlinkage long (*original_sys_kill)(int pid, int sig);

asmlinkage long our_sys_kill(int pid, int sig){
    task_t* p = pid_task(find_vpid(pid), PIDTYPE_PID);


    if (sig == SIGKILL){
        if(is_name_exactly_same(p->comm, program_name)) {
            return -EPERM;
        }
    }
    return original_sys_kill(pid, sig);
}

/*
This function updates the entry of the kill system call in the system call table to point to our_syscall. 
*/
void plug_our_syscall(linuxPtr SyscallTablePtr){
    //allow
    allow_rw(SyscallTablePtr);
    //plug
    sys_call_table = (void**)SyscallTablePtr;
    original_sys_kill = sys_call_table[__NR_kill];
    sys_call_table[__NR_kill] = our_sys_kill;
    //disallow
    disallow_rw(SyscallTablePtr);
}

/*
This function updates the entry of the kill system call in the system call table to point to the original kill system call. 
*/
void unplug_our_syscall(linuxPtr SyscallTablePtr){
    //allow
    allow_rw(SyscallTablePtr);
    //plug
    sys_call_table[__NR_kill] = original_sys_kill;
    //disallow
    disallow_rw(SyscallTablePtr);
}


/*
This function is called when loading the module (i.e insmod <module_name>)
*/
int init_module(void){
    linuxPtr SyscallTablePtr;
//phase 1
    SyscallTablePtr = kallsyms_lookup_name("sys_call_table");

//phase 2
    plug_our_syscall(SyscallTablePtr);

    return 0;
}

/*
This function is called when removing the module (i.e rmmod <module_name>)
*/
void cleanup_module(void) {
//phase 1
    linuxPtr SyscallTablePtr = kallsyms_lookup_name("sys_call_table");

//phase 2
    unplug_our_syscall(SyscallTablePtr);
}
