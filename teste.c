#include <stdio.h>


#define COMMANDS_TABLE \
  X(COMMAND_TODAY, "CommandToday") \
  X(COMMAND_HELP, "CommandHelp") \
  X(COMMAND_SETUP, "CommandSetup")\
  X(COMMAND_TEST, "CommandTest")

#define X(type,string) type,
typedef enum{
   COMMANDS_TABLE
}Command;
#undef X


void command_handle(Command cmd){

#define X(type, string) \
       case type: {printf("%s\n\r",string); }break;

     switch(cmd){
        COMMANDS_TABLE

     }  
     
#undef X

}


int main(void){

   Command cmd = COMMAND_SETUP;
   printf ("resul %d \n\r",cmd);

   command_handle(cmd);

   return 0;


}