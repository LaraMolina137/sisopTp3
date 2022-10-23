#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
//#include "process.h"

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
    pid_t pid;

    pid = fork();

    if(pid > 0) { //padre

        Process father(getpid(), getppid(), 1, "padre");
        father.message();

        pid = father.createSon();
        if(pid > 0) {
            int status;
            pid_t w = waitpid(pid, &status, WUNTRACED);
        }
        if(pid == 0) {  //hijo 2
            Process son(getpid(), getppid(), 2, "hijo");
            son.message();

            pid = son.createSon();

            if(pid > 0) {
                int status;
                pid_t w = waitpid(pid, &status, WUNTRACED);
            }
            if(pid == 0) {
                 Process grandSon(getpid(), getppid(), 3, "nieto");
                grandSon.message();
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
                Process grandSon(getpid(), getppid(), 3, "nieto");
                grandSon.message();
            }

        }else if(pid == 0) {
            Process grandSon(getpid(), getppid(), 3, "nieto");
            grandSon.message();
        }

    }
   
    return EXIT_SUCCESS;
}