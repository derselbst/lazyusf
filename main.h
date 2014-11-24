#include <stdint.h>

#define CPU_Interpreter             1
#define CPU_Recompiler              0

int InitalizeApplication ( void );
void DisplayError (char * Message, ...);
void StopEmulation(void);
void UsfSleep(int32_t);
