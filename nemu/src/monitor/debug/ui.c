#include <isa.h>
#include "expr.h"
#include "watchpoint.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);
int is_batch_mode();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);
 
//单步执行
static int cmd_si(char *args){
  if (args == NULL){
    cpu_exec(1);
    return 0;
  }else{
    int n=0;
    while( sscanf(args,"%d",&n) != EOF){
      cpu_exec(n);
      return 0;
    };
    printf("Input format error!");
  }
}

static void print_register(){
  for(int i=0;i<8;i++){
    printf("%s\t%#012X\t%d\n",reg_name(i,4),reg_l(i),reg_l(i));
  }
  for(int i=0;i<8;i++){
    printf("%s\t%#012X\t%d\n",reg_name(i,2),reg_w(i),reg_w(i));
  } 
  for(int i=0;i<8;i++){
    printf("%s\t%#012X\t%d\n",reg_name(i,1),reg_b(i),reg_b(i));
  }
}

//打印寄存器
static int cmd_info(char *args){
  if(args == NULL){
    printf("Input format error!");
    return 0;
  }
  switch(*args){
    case 'r':
      print_regster();
      break;
    case 'w':

      break;
  }
}

//扫描内存
static int cmd_scan(int num,char *str){

}

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  {"si", "Single instruction execuiton", cmd_si },      //单步执行
  {"info", "Print the registers or watchpoints", cmd_info },    //打印寄存器状态或者监视点信息
  {"x", "Scan the memory", cmd_scan }       //扫描内存
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop() {
  if (is_batch_mode()) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
