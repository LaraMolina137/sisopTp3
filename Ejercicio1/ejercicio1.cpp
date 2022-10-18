#include <iostream>
#include <sys/wait.h>
#include <unistd.h>


using namespace std;

int main()
{
    pid_t ppid, c1pid, c2pid;
    c1pid = fork();
  
     if (c1pid == 0) {

        wait(nullptr);

    } else {//hijo

        cout << "soy pid hijo: " << getpid() <<endl;
        cout << "padre : " << getppid() <<endl;

        c2pid = fork();

        if(c2pid == 0){
             wait(nullptr);
        }else{ //nieto
            cout << "soy pid hijo: " << getpid() <<endl;
            cout << "padre : " << getppid() <<endl;
        }
        exit(EXIT_SUCCESS);
    }

    return EXIT_SUCCESS;
}
