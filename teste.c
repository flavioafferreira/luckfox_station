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

/* RESULTADO
typedef enum{
   COMMAND_TODAY, COMMAND_HELP, COMMAND_SETUP, COMMAND_TEST,
}Command;
*/

void command_handle(Command cmd){

#define X(type, string) \
       case type: {printf("%s\n\r",string); }break;

     switch(cmd){
        COMMANDS_TABLE

     }  
     
#undef X

}

/*
//RESULTADO
gcc -E -P teste.c | grep case
 case COMMAND_TODAY: {printf("%s\n\r","CommandToday"); }break; 
 case COMMAND_HELP: {printf("%s\n\r","CommandHelp"); }break; 
 case COMMAND_SETUP: {printf("%s\n\r","CommandSetup"); }break;
 case COMMAND_TEST: {printf("%s\n\r","CommandTest"); }break;
*/


int main(void){

   Command cmd = COMMAND_SETUP;
   printf ("resul %d \n\r",cmd);

   command_handle(cmd);

   return 0;


}