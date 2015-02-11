#include <stdint.h>
#include <linux/limits.h>

#define CPU_Interpreter             1
#define CPU_Recompiler              0

extern char filename[PATH_MAX];

void DisplayError (char * Message, ...);
void StopEmulation(void);
