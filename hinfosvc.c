#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define ERROR(msg) do{fprintf(stderr, "%s\n", msg); exit(1);} while(0)

#define NEXT_NUM(str) while(*str < 48 || *str > 57) str++

//Get the hostname from a system file
void getName(char *name){
  FILE *f = fopen("/proc/sys/kernel/hostname", "r");
  if(!f)
    ERROR("Unable to read hostname");
  fgets(name, 127, f);
  fclose(f);
}

//Get the cpu name from a system file
void getCpu(char *cpu){
  FILE *f = fopen("/proc/cpuinfo", "r");
  if(!f)
    ERROR("Unable to read cpuinfo");
  char line[256];
  while(1){
    fgets(line, 255, f);
    if(strncmp(line, "model name", strlen("model name")) == 0){
      int i = strlen("model name") + 1;
      while(line[i] == ' ' || line[i] == ':')
        i++;
      strcpy(cpu, line + i);
      fclose(f);
      return;
    }
  }
}

//Get the cpu load
void getLoad(char *load){
  FILE *f = fopen("/proc/stat", "r");
  if(!f)
    ERROR("Unable to read load data");

  char *ptr = malloc(256);
  char *line = ptr;
  if(!line)
    ERROR("Malloc failed");
  unsigned long prev_loads[8];
  fgets(line, 255, f);
  NEXT_NUM(line);

  for(int i = 0; i < 8; i++){
    //char *val = strtok(line, ' ');
    prev_loads[i] = strtoul(line, &line, 10);
    NEXT_NUM(line);
  }
  fclose(f);
  sleep(1);
  
  f = fopen("/proc/stat", "r");
  if(!f)
    ERROR("Unable to read load data");

  line = ptr;
  unsigned long loads[8];
  fgets(line, 255, f);
  NEXT_NUM(line);

  for(int i = 0; i < 8; i++){
    //char *val = strtok(line, ' ');
    loads[i] = strtoul(line, &line, 10);
    NEXT_NUM(line);
  }
  fclose(f);
  free(ptr);

  //https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux
  unsigned long PrevIdle = prev_loads[3] + prev_loads[4];
  unsigned long Idle = loads[3] + loads[4];

  unsigned long PrevNonIdle = prev_loads[0] + prev_loads[1] + prev_loads[2] + prev_loads[5] + prev_loads[6] + prev_loads[7];
  unsigned long NonIdle = loads[0] + loads[1] + loads[2] + loads[5] + loads[6] + loads[7];

  unsigned long PrevTotal = PrevIdle + PrevNonIdle;
  unsigned long Total = Idle + NonIdle;

  unsigned long totald = Total - PrevTotal;
  unsigned long idled = Idle - PrevIdle;

  double CPU_Percentage = (totald - idled) / (double) totald * 100;

  sprintf(load, "%.0lf%%", CPU_Percentage);
}

int main(int argc, char** argv){
  if(argc != 2)
    ERROR("Usage: ./hinfosvc PORT");

  //Parse port
  char *endptr;
  int port = strtoul(argv[1], &endptr, 10);
  if(strcmp(endptr, "") || port == 0)
    ERROR("Error parsing port");

  //Create socket
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if(server_fd < 0) 
    ERROR("Failed to create a socket");

  //Set socket options
  int opt = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(int));

  //Bind socket to port and listen
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  if(bind(server_fd, (struct sockaddr*) &addr, sizeof(addr)) < 0) 
    ERROR("Failed to bind the socket");
  
  if(listen(server_fd, 1) < 0)
    ERROR("Failed to listen on socket");

  while(1){
    //Accept request
    int client_fd = accept(server_fd, NULL, NULL);
    if(client_fd < 0)
      ERROR("Failed to accept connection");

    char in[1024];
    int size = read(client_fd, in, 1023);
    in[1023] = 0;

    //Compare request
    char headers[] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n";
    char data[128] = "";
    if(strncmp(in, "GET /hostname ", strlen("GET /hostname ")) == 0)
      getName(data);
    else if(strncmp(in, "GET /cpu-name ", strlen("GET /cpu-name ")) == 0)
      getCpu(data);
    else if(strncmp(in, "GET /load ", strlen("GET /load ")) == 0)
      getLoad(data);
    else
      strcpy(headers, "HTTP/1.1 400 Bad Request");

    //Consume rest of data in socket
    if(size == 1023) 
      while(read(client_fd, in, 1023) == 1023);

    //Send the data
    if(send(client_fd, headers, strlen(headers), 0) < 0 || send(client_fd, data, strlen(data), 0) < 0 )
      ERROR("Failed to send data");
    
    close(client_fd);
  }
  return 0;
}