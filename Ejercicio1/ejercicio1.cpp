#include <iostream>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;

class Process
{
    public:
        pid_t   pid;
        pid_t   ppid;
        int     numberGeneration;
        char*   relationShip;
    
        Process(){

        }
        Process(pid_t pid, pid_t ppid, int numberGeneration, char* relationShip)
        {
            this->pid = pid;
            this->ppid = ppid;
            this->numberGeneration = numberGeneration;
            this->relationShip = relationShip;
        }
         
        ~Process()
        {
           
        }


        void message() {
//Soy el proceso con PID ...... y pertenezco a la generación No ....... Pid: ......... Pid padre: ..... Parentesco/Tipo: [nieto, hijo, zombie]
            cout << "Soy el proceso con PID " << pid 
            << " y pertenezco a la generación No " << numberGeneration 
            << " Pid: " << pid << " Pid padre: " << ppid << " Parentesco/Tipo: " << relationShip <<endl;
        }

        pid_t createSon(){
            return fork();
        }
        
};

int main()
{
    pid_t pid, pidz;

    pid = fork();

    if(pid > 0) { //padre

        Process father(getpid(), getppid(), 1, "padre/proceso");
        father.message();

        pid = father.createSon();
        if(pid > 0) {
            int status;
            pid_t w = waitpid(pid, &status, WUNTRACED);
        }
        if(pid == 0) {  //hijo 2
            Process son(getpid(), getppid(), 2, "hijo/proceso");
            son.message();

            pid = son.createSon();

            if(pid > 0) {
                int status;
                pid_t w = waitpid(pid, &status, WUNTRACED);
            }
            if(pid == 0) {
                Process grandSon(getpid(), getppid(), 3, "nieto/proceso");
                grandSon.message();

                pid = fork();
                if(pid > 0){
                    int status;
                    pid_t w = waitpid(pid, &status, WUNTRACED);
                } else {
                    Process greatGrandSon(getpid(), getppid(), 4, "bisnieto/proceso");
                    greatGrandSon.message();
                }
            }
        }

    }
    else if (pid == 0) { //hijo 1
        pid_t p = getppid();
        Process son(getpid(), p, 2, "hijo");
        son.message();

        pid = son.createSon();

        if(pid > 0) {
            pid = son.createSon();
            
            if(pid > 0) {
                    int status;
                    pid_t w = waitpid(pid, &status, WUNTRACED);
            } else if(pid == 0) {
                Process grandSon(getpid(), getppid(), 3, "nieto/proceso");
                grandSon.message();

                pid = fork();
                if(pid > 0) {
                    pid = fork();

                    if(pid > 0) {
                        int status;
                        pid_t w = waitpid(pid, &status, WUNTRACED);
                    }
                    else if(pid == 0) {
                        Process greatGrandSon(getpid(), getppid(), 4, "bisnieto/proceso");
                        greatGrandSon.message();
                    }

                }
                else if(pid == 0) {
                    Process greatGrandSon(getpid(), getppid(), 4, "bisnieto/proceso");
                    greatGrandSon.message();
                }
            }

        }else if(pid == 0) {
            Process grandSon(getpid(), getppid(), 3, "nieto/proceso");
            grandSon.message();

            pid = fork();
                if(pid > 0) {
                    pid = fork();

                    if(pid > 0) {
                        while (true)
                        {
                            /* code */
                        }
                        
                    }
                    else if(pid == 0) {
                        Process greatGrandSon(getpid(), getppid(), 4, "bisnieto/zombie");
                        greatGrandSon.message();
                        exit(0);

                    }

                }
                else if(pid == 0) {
                    Process greatGrandSon(getpid(), getppid(), 4, "bisnieto/zombie");
                    greatGrandSon.message();

                    //daemon
                   pid = fork();
                    if(pid > 0){
                        exit(0);
                    }else if (pid == 0){
                        pid = fork();
                        if(pid > 0){
                             Process daemon(getpid(), 1, 5, "demonio");
                            daemon.message();
                            while (true){}
                        }

                    }
                }

        }
    
    }

    

         while(true)
                {
                    /* code */
                }  

        return EXIT_SUCCESS;
}