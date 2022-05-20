#include<iostream>
using namespace std;
#include<unistd.h>
#include<sys/wait.h>
#include<cstring>
#include<string>
#include<vector>
#include<fcntl.h>

const int std_write = dup(1);
const int std_read = dup(0);
void reset_std(){
    dup2(std_read,0);
    dup2(std_write,1);
}
int oldpipe[2];
int newpipe[2];
int file;

// Utility Function
void CallCommand(vector<char*> &args);

void CMD(bool &flag){
    string input;
    cout << "\033[1;4;36mEnter a Command:\033[0m\n";
    getline(cin,input);
    if(input == "exit" || input == "Exit"){
        flag = true;
        return;
    }
    // convert string to char*
    char* str = new char[input.size()];
    strcpy(str, input.c_str());

    char* token = strtok(str," ");
    vector<char*> args; 
    
    //oldpipe = new int[2];
    oldpipe[0] = std_read;
    oldpipe[1] = std_write;
    //newpipe = new int[2];
    newpipe[0] = std_read;
    newpipe[1] = std_write;
    
    int curr_read , curr_write;
    
    bool found_pipe = false;
    bool is_file_read = false;
    bool is_file_write = false;

    while(token != NULL){
        //cout << "Token = " << token << endl;
        if(strchr(token,'|')){ // make pipe
                if(pipe(newpipe) == -1){
                    dprintf(std_write,"\033[4;1;31mError in Pipe\033[0m\n");
                }
                
                // redirect to pipe
                if(dup2(oldpipe[0],0) == -1){
                    dprintf(std_write,"\033[4;1;31mError in Dup\033[0m\n");
                }
                curr_read = oldpipe[0];
                //dprintf(std_write,"\033[1;4;32moldpipe[0] -> std_read\033[0m\n");

                if(dup2(newpipe[1],1) == -1){
                    dprintf(std_write,"\033[4;1;31mError in Dup\033[0m\n");
                }
                curr_write = newpipe[1];
                //dprintf(std_write,"\033[1;4;32mnewpipe[1] -> std_write\033[0m\n");

                
                found_pipe = true;
                CallCommand(args);
        }
        else if(strchr(token,'>')){ // output to file
            token = strtok(NULL," ");
            file = open(token,O_WRONLY | O_CREAT,0777);
            if(file == -1){
                dprintf(std_write,"\033[4;1;31mError in Opening File\033[0m\n");
            }
            dup2(file,1);
            is_file_write = true;
            curr_write = file;
        }
        else if(strchr(token,'<')){ // input from file
            token = strtok(NULL," ");
            file = open(token,O_RDONLY,0777);
            if(file == -1){
                dprintf(std_write,"\033[4;1;31mError in Opening File\033[0m\n");
            }
            dup2(file,0);
            is_file_read = true;
            curr_read = file;
        }
        else{
            args.push_back(token);
        }
        oldpipe[0] = newpipe[0];
        oldpipe[1] = newpipe[1];
        token = strtok(NULL," ");
    }

    if(found_pipe){
        if(!is_file_read){
            if (dup2(oldpipe[0],0) == -1){
                dprintf(std_write,"\033[4;1;31mError in Dup2\033[0m\n");
            }
            curr_read = oldpipe[0];
        }
        else{
            is_file_read = false;
        }
        if(!is_file_write){
            if (dup2(std_write,1) == -1){
                dprintf(std_write,"\033[4;1;31mError in Dup2\033[0m\n");
            }
            curr_write = std_write;
        }
        else{
            is_file_write = false;
        }   
    }
    CallCommand(args);
}

void CallCommand(vector<char*> &args){
    int pid = fork();
    if(pid == 0){
        args.push_back(NULL);
        if(close(newpipe[0]) == -1){
            //dprintf(std_write,"\033[4;1;31mError in Close\033[0m\n");
        }
        char** command = &args[0];
        if(execvp(command[0],command) == -1){
            dprintf(std_write,"\033[4;1;31mError in Exec \nInvalid Command\033[0m\n");
        }
    }
    else if(pid > 0){
        if(newpipe[1] != std_write){
            if(close(newpipe[1]) == -1){
                //dprintf(std_write,"\033[4;1;31mError in Close\033[0m\n");
            }
        }
        if(wait(NULL) == -1){
            dprintf(std_write,"\033[4;1;31mError in Wait\033[0m\n"); 
        }
        args.clear();
    }
    else{
        dprintf(std_write,"\033[4;1;31mError in Fork\033[0m\n");
    }
}

int main(){
    cout << "\033[1;4;33m\t\tWelcome to Lazy Terminal\033[0m\n";
    bool flag = false;
    do{
        dup2(std_read, 0);
        dup2(std_write, 1);
        CMD(flag);
        if(flag){
            break;
        }
    }while(!flag);
    cout << "\033[1;4;32m\t\tGood Bye Have a Nice day\033[0m\n";
    return 0;
}