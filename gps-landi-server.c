/**
** GPS SERVER FOR RASPBERRY PI BOARD + ULTIMATE GPS (https://www.adafruit.com/product/746)
** it listens on port tcp/8000 for location request ->GPS answer with XXXX.XXXXX#YYYY.YYYYY#Z where X is latitude and Y is longitude Z is the seconds from the last reading.
**/
#include <stdio.h>   /* Standard library definition */
#include <stdlib.h>  /* Standard library definition */
#include <string.h>  /* Standard library definition */
#include <fcntl.h>   /* File Control Definitions           */
#include <termios.h> /* POSIX Terminal Control Definitions */
#include <unistd.h>  /* UNIX Standard Definitions 	   */ 
#include <errno.h>   /* ERROR Number Definitions           */
#include <pthread.h> /* threads definitions		   */
#include <sys/socket.h> /* socket definitions		   */ 
#include <sys/types.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <syslog.h>

// FUNCTION DEFINITION
void landi_exit_error(char *);
int landi_open_rs232(char *);
void landi_close_rs232(int);
void * landi_read_rs232(void *);
void landi_write_log(char *);
//*** CUSTOMIZATION PARAMETERS
#define RS232 "/dev/ttyS0"
//*** GLOBAL VARIABLES
char umapos[128];

/* main function */
void main(void){
    int rs232=0;
    pthread_t t;
    long nt=0;
    int rc=0;
    struct sockaddr_in servaddr, cli; 
    int sockfd,connfd,len;
    char clientip[256];
    //OPEN SERIAL LINE
    rs232=landi_open_rs232(RS232); 
    // LAUNCH THREAD TO READ SERIAL LINE WITH GPS POSITION IN GLOBAL VAR umapos
    rc=pthread_create(&t,NULL,landi_read_rs232,(void *) rs232);    
    // SERVER SOCKET PORT 8000 LISTENING FOR REQUEST
    sockfd = socket(AF_INET, SOCK_STREAM, 0);  /** SOCKET CREATION 	*/    
    if (sockfd == -1) landi_exit_error("1003 - Error creating socket");
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(8000);
    //bzero(&(servaddr.sin_zero),8); 
    if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) landi_exit_error("1004 - Error binding socket");
    if ((listen(sockfd, 100)) != 0) landi_exit_error("1005 - Error listening to socket");
    while(1){
          len=sizeof(cli);
          printf("uma-gps-server.c: Server Listening on port 8000\n");
          connfd = accept(sockfd, (struct sockaddr *)&cli, &len);     
          if(connfd<0) landi_exit_error("1006 - Error accepting connection to socket"); 
          strncpy(clientip, inet_ntoa(cli.sin_addr),256);  
          printf("uma-gps-server.c: connection from: %s \n",clientip);        
          printf("uma-gps-server.c: Sending: %s \n",umapos);
          write(connfd, umapos, strlen(umapos));
          shutdown(connfd,SHUT_RDWR);
          close(connfd);
    }
    //CLOSE SERIAL LINE
    landi_close_rs232(rs232);
    exit(0);
}
/**
** FUNCTION TO READ DATA
**/
void * landi_read_rs232(void *fd){
    char buffer[512];
    char buf[64];
    int lenbuf=0;
    int rs232=(int)fd;
    while(1){
        memset(buffer,0,512);
        memset(buf,0,64);
        lenbuf=read(rs232,buffer,80);
        strncpy(buf,buffer,6);
        if(strcmp(buf,"$GPRMC")==0){
          strncpy(umapos,&buffer[7],lenbuf-11);   
          umapos[lenbuf-11]=0;
          landi_write_log(buffer);
        }
    }

}

/**
** FUNCTION TO OPEN SERIAL LINE RS-232
**/
int landi_open_rs232(char * dev){
    int fd=0;
    fd = open(dev,O_RDWR | O_NOCTTY);
    if(fd==-1)
         landi_exit_error("1000 - Error opening serial line");
    struct termios SerialPortSettings; 	/* Create the structure                          */
    tcgetattr(fd, &SerialPortSettings);	/* Get the current attributes of the Serial port */
    /* Setting the Baud rate */
    cfsetispeed(&SerialPortSettings,B9600); /* Set Read  Speed as 9600                       */
    cfsetospeed(&SerialPortSettings,B9600); /* Set Write Speed as 9600                       */
    /* 8N1 Mode */
    SerialPortSettings.c_cflag &= ~PARENB;   /* Disables the Parity Enable bit(PARENB),So No Parity   */
    SerialPortSettings.c_cflag &= ~CSTOPB;   /* CSTOPB = 2 Stop bits,here it is cleared so 1 Stop bit */
    SerialPortSettings.c_cflag &= ~CSIZE;	 /* Clears the mask for setting the data size             */
    SerialPortSettings.c_cflag |=  CS8;      /* Set the data bits = 8                                 */
    SerialPortSettings.c_cflag &= ~CRTSCTS;  /* No Hardware flow Control                         */
    SerialPortSettings.c_cflag |= CREAD | CLOCAL; /* Enable receiver,Ignore Modem Control lines       */ 
    SerialPortSettings.c_iflag &= ~(IXON | IXOFF | IXANY);          /* Disable XON/XOFF flow control both i/p and o/p */
    SerialPortSettings.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);  /* Non Cannonical mode                            */
    SerialPortSettings.c_oflag &= ~OPOST;/*No Output Processing*/
    /* Setting Time outs */
    SerialPortSettings.c_cc[VMIN] = 10; /* Read at least 10 characters */
    SerialPortSettings.c_cc[VTIME] = 0; /* Wait indefinetly   */
    if((tcsetattr(fd,TCSANOW,&SerialPortSettings)) != 0) /* Set the attributes to the termios structure*/{
         close(fd);
         landi_exit_error("1002 - Error setting rs232 parameters");
    }
    tcflush(fd, TCIFLUSH);   /* Discards old data in the rx buffer            */
    return(fd);
}
/**
** FUNCTION TO CLOSE SERIAL LINE RS-232
**/
void landi_close_rs232(int fd){
    if(close(fd)==-1)
        landi_exit_error("1010 - Error closing serial line");
    return;
}
/**
** FUNCTION TO PRINT/LOG THE ERROR AND EXIT 
**/
void landi_exit_error(char * error){
    puts(error);
    if(errno!=0) printf("errno: %d - %s",errno,strerror(errno));
    
    exit(-1);
}	
/**
** FUNCTION TO WRITE TO LOG
**/
void landi_write_log(char *msg){
openlog("uma-gps-server",LOG_USER,LOG_PID);
syslog(LOG_INFO,msg);
closelog();
}