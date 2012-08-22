#include "unrar.h"

#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/wait.h>
#include <getopt.h>
#define LENGTH 6;

const char* file;
char *tab;
bool found;
unsigned int length;


static int 
_unrar_test_password_callback(unsigned int msg, long data, long P1, long P2)
{
    switch(msg) {
          case UCM_NEEDPASSWORD:
            *(int *)data = 1;
          break;
          default:
            return 0;
    }
    return 0;
}


static int
unrar_test_password(const char * file, const char * pwd) 
{
    void                      * unrar;
    struct RARHeaderData        headerdata;
    struct RAROpenArchiveData   archivedata;
    int                         password_incorrect;
    
    password_incorrect = 0;
    memset(&headerdata,0, sizeof(headerdata));
    memset(&archivedata,0, sizeof(archivedata));
    archivedata.ArcName  = (char *) file;
    archivedata.OpenMode = RAR_OM_EXTRACT;
    
    unrar = RAROpenArchive(&archivedata);
    if (!unrar || archivedata.OpenResult)
        return -2;
    RARSetPassword(unrar, (char *) pwd);
    
    RARSetCallback(unrar, _unrar_test_password_callback, (long) &password_incorrect);
    
    if (RARReadHeader(unrar, &headerdata) != 0)
        goto error;
    
    if (RARProcessFile(unrar, RAR_TEST, NULL, NULL) != 0)
        goto error;
    
    if (password_incorrect)
        goto error;
    RARCloseArchive(unrar);
    return 0;
    
error:
    RARCloseArchive(unrar);
    return -1;
}

void
usage(void)
{
    printf("Usage: rarcrack [-p nbProcs | -t nbThreads ] [-l length] [-c testing characters ] RAR_FILE \n");
}


void 
genAndTest(char *l, unsigned int lLength,unsigned int len){
    int i;
        for(i=0; i<strlen(tab);i++){
                char* pass = malloc(lLength+2);         /* Allocating mem for the pass */
                if(pass == NULL){
                    perror("ERROR malloc failed\n");
                    exit(-1);
                }
                strncpy(pass,l,lLength);            /* copiying the prefix l in pass */
                pass[lLength] = tab[i];             /* adding the new letter */
                if(unrar_test_password(file, pass)==0){
                    printf("password is :%s\n", pass);      /* Password found */
                    found = true;
                    free(pass);
                    exit(1);
                }
                free(pass);	
        }
        
        for(i=0; i<strlen(tab); i++){
            if(len-1>1){
                        char *temp = malloc(lLength*sizeof(char)+2);
                        if(temp == NULL){
                            perror("ERROR malloc failed\n");
                            exit(-1);
                        }
                        strncpy(temp,l,lLength);
                        temp[lLength] = tab[i];
                        temp[lLength+1] = '\0';     
                        genAndTest(temp,lLength+1,len-1);       /* recursive call for generating pass of size length+1 */
                        free(temp);
                }
        }
}
void
*runInThread(void* p){
    int i;
        char *domain;
            domain = (char*)p;
                for(i=0; i< strlen(domain);i++){                /* for each letter in the domain generate the possible combinaison of pass and test it*/
                    genAndTest(&domain[i],1,length);
                }
            free(domain);
            pthread_exit(NULL);
}

void
runInProc(char* domain){
    int i;
        for(i=0; i< strlen(domain);i++){                    /* for each letter in the domain generate the possible combinaison of pass and test it*/
            genAndTest(&domain[i],1,length);
        }
    free(domain);
}

void
executeProc(int nbProc){
    int pId;
    int nbElemForEach;
    int lastIndex;
    int i;
    int *fd;
    char ** domain;
        fd = malloc(nbProc*sizeof(int));
        pipe(fd);
        nbElemForEach = (strlen(tab)/nbProc)+1;
        lastIndex = 0;
        domain = malloc(sizeof(char*)*nbProc);          /* finding the domain for the thread i.e divinding the alphabet by nb procs */
        if(domain == NULL){
            perror("ERROR malloc failed\n");
            exit(-1);
        }
        for(i=0; i<nbProc; i++){
            domain[i] = malloc(sizeof(char)*nbElemForEach);
            if(domain[i] == NULL){
                perror("ERROR malloc failed\n");
                free(domain);
                exit(-1);
            }
            strncpy(domain[i],tab+lastIndex,nbElemForEach);         /* setting the char in the array . This array is the domain , it contains the char that the thread will test*/
            lastIndex+=nbElemForEach;
            if(strlen(domain[i])>0){
                pId= fork();                                    /* forking the process */
                if (pId == 0){ /*Child process */
                    runInProc(domain[i]);                       /* call the generatePass function */
                    free(domain);
                    exit(-1);
                }else if(pId<0){/* ERROR */
                    perror("ERROR: cannot create child process");
                    free(domain[i]);
                    free(domain);
                    exit(-1);
                }
            }
        }
          
        wait(0);
	   /*kill(getppid(),SIGKILL);*/
       free(domain);
}


void
executeThread(int nbThreads){
 
    pthread_t *threads;  /*Array of threads*/
    int rc;             
    int nbElemForEach;
    int lastIndex;
    int i;
    char ** domain;
    
        threads = malloc(sizeof(pthread_t)*nbThreads);
        if(threads == NULL){
            perror("ERROR malloc failed");
            exit(-1);
        }
        nbElemForEach = (strlen(tab)/nbThreads)+1;      /*Nb of char for each thread*/
        lastIndex = 0;
        domain = malloc(sizeof(char*)*nbThreads);
        if(domain == NULL){
            perror("ERROR malloc failed\n");
            free(threads);
            exit(-1);
        }
           
        for(i=0; i<nbThreads; i++){
            domain[i] = malloc(sizeof(char)*nbElemForEach); 
            if(domain[i] == NULL){
                perror("ERROR malloc failed\n");
                free(domain);
                free(threads);
                exit(-1);
            }
            strncpy(domain[i],tab+lastIndex,nbElemForEach);/* setting the char for the thread*/
            lastIndex+=nbElemForEach;
            rc = pthread_create(&threads[i],NULL,runInThread,(void*)domain[i]);   /*Creating thread*/
            if (rc){
                printf("ERROR; return code from pthread_create() is %d\n", rc);
                free(domain[i]);
                free(domain);
                free(threads);
                exit(-1);
            }
            
        }
        for(i=0; i<nbThreads; i++)
			pthread_join(threads[i], NULL);
        pthread_exit(NULL);
        free(domain);
        free(threads);
}

void checkParams(char* pflag, char* tflag){
    if(pflag!=NULL && tflag!=NULL){
        usage();
        exit(-1);
    }else if(pflag==NULL && tflag==NULL){
        usage();
        exit(-1);
    }else if(file == NULL){
        usage();
        exit(-1);
    }
}

void
execute(char* pflag, char* tflag){
    if(pflag != NULL)               /*if it's a proc*/
        executeProc(atoi(pflag));
    else
        executeThread(atoi(tflag)); /*else it's a thread*/
}

int 
main (int argc, char **argv)
{
    int c;
    char* pflag = 0;
    char* tflag = 0;
    char* cflag = 0;
    char* lflag = 0;
  
        found = false;
        if (argc < 2) {
            usage();
            return 1;
        }
        
        while ((c = getopt (argc, argv, "c:l:p:t:")) != -1)	{  /*Parse arguments*/
            switch (c){
                case 'p':
                    pflag = (char*)optarg;
                    break;
                case 't':
                    tflag = (char*)optarg;
                    break;
                case 'c':
                    cflag = (char*)optarg;
                    break;
                case 'l':
                    lflag = (char*)optarg;
                    break;
            }
        }
        if(lflag == NULL){
            length = LENGTH;
        }else length = atoi(lflag);
        if(cflag == NULL)
            tab = "abcdefghijklmnopqrstuvwxyz";
        else
            tab = cflag;
        file = argv[argc-1];
        checkParams(pflag, tflag); /*Exits if arguments are wrong*/
        printf("Starting pass cracking on file %s\nMax pass length: %d\nCharacters: %s\n",file, length, tab);
        execute(pflag, tflag); /*Starts cracking the pass with threads or procs*/
        return 1;  
}
