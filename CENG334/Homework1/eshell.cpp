#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>
#include "parser.c"

void singleInput(single_input input){
    int status;
    pid_t pid;
    if(pid = fork()){ // parent
        waitpid(pid, &status, 0);
    }
    else{ // child
        execvp(input.data.cmd.args[0], input.data.cmd.args);
    }
}

void pipedInput(parsed_input input){
    int i, status;
    int size = input.num_inputs;
    pid_t pids[size];
    int pipes[size-1][2];
    for(i = 0 ; i < size ; i++){
        if(i == 0){
            pipe(pipes[i]);
            if(pids[i] = fork()){ // parent
                
            }
            else{
                close(pipes[i][0]);
                dup2(pipes[i][1], 1);
                close(pipes[i][1]);
                if(input.inputs[i].type != INPUT_TYPE_SUBSHELL) execvp(input.inputs[i].data.cmd.args[0], input.inputs[i].data.cmd.args);
            }
        }
        else if(i == size-1){
            if(pids[i] = fork()){
                int k;
                for(k = 0 ; k < i ; k++){
                    close(pipes[k][0]);
                    close(pipes[k][1]);
                }
                for(k = 0 ; k <= i ; k++){
                    waitpid(pids[k], &status, 0);
                }
            }
            else{
                int k;
                dup2(pipes[i-1][0], 0);
                for(k = 0 ; k < i ; k++){
                    close(pipes[k][0]);
                    close(pipes[k][1]);
                }
                if(input.inputs[i].type != INPUT_TYPE_SUBSHELL) execvp(input.inputs[i].data.cmd.args[0], input.inputs[i].data.cmd.args);
            }
        }
        else{
            pipe(pipes[i]);
            if(pids[i] = fork()){ // parent
                
            }
            else{
                int k;
                dup2(pipes[i-1][0], 0);
                dup2(pipes[i][1], 1);
                for(k = 0 ; k <= i ; k++){
                    close(pipes[k][0]);
                    close(pipes[k][1]);
                }
                if(input.inputs[i].type != INPUT_TYPE_SUBSHELL) execvp(input.inputs[i].data.cmd.args[0], input.inputs[i].data.cmd.args);
            }
        }
    }
}

void plineToParsedInput(pipeline pline, parsed_input* newParsed){
    int i, plineSize = pline.num_commands;
    for(i = 0 ; i < plineSize ; i++){
        single_input element;
        element.data.cmd = pline.commands[i];
        newParsed->inputs[i] = element;
        newParsed->num_inputs++;
    }
}

void sequentialInput(parsed_input input){
    int i, status;
    int size = input.num_inputs;
    pid_t pid[size];
    for(i = 0 ; i < size ; i++){
        if(pid[i] = fork()){
            waitpid(pid[i], &status, 0);
        }
        else{
            if(input.inputs[i].type == INPUT_TYPE_PIPELINE){
                parsed_input newParsed;
                plineToParsedInput(input.inputs[i].data.pline, &newParsed);
                pipedInput(newParsed);
            }
            else{
                singleInput(input.inputs[i]);
            }
            exit(0);
        }
    }
}

void parallelInput(parsed_input input){
    int status;
    int size = input.num_inputs;
    pid_t pid[size];
    int i;
    for(i = 0 ; i < size ; i++){
        pid[i]=fork();
        if(!pid[i]){
            if(input.inputs[i].type == INPUT_TYPE_PIPELINE){
                parsed_input newParsed;
                plineToParsedInput(input.inputs[i].data.pline, &newParsed);
                pipedInput(newParsed);
            }
            else{
                singleInput(input.inputs[i]);
            }
            exit(0);
        }
    }
    for(i = 0 ; i < size ; i++) waitpid(pid[i], &status, 0);
}

// void subshellParallelInput(parsed_input input){

// }

void repeater(parsed_input parallel){
    int i, status, size = parallel.num_inputs;
    int pipes[size][2];
    pid_t pid[size+1];
    for(i = 0 ; i < size ; i++){
        pipe(pipes[i]);
    }
    for(i = 0 ; i < size ; i++){
        if(pid[i] = fork()){

        }
        else{
            int k;
            dup2(pipes[i][0], 0);
            
            for(k = 0 ; k < size ; k++){
                close(pipes[k][0]);
                close(pipes[k][1]);
            }

            if(parallel.inputs[i].type == INPUT_TYPE_PIPELINE){
                parsed_input newParsed;
                plineToParsedInput(parallel.inputs[i].data.pline, &newParsed);
                pipedInput(newParsed);
                exit(0);
            }
            else execvp(parallel.inputs[i].data.cmd.args[0], parallel.inputs[i].data.cmd.args);
        }
    }
    if(!(pid[size] = fork())){
        char buf[1];
        while(read(0, buf, sizeof(buf))){
            for(i = 0 ; i < size ; i++){
                write(pipes[i][1], buf, sizeof(buf));
            } 
        }
        // read(0, buf, sizeof(buf));
        // printf("buf: %s\n", buf);
        for(i = 0 ; i < size ; i++){
            close(pipes[i][0]);
            close(pipes[i][1]);
        }
    }
    for(i = 0 ; i < size ; i++){
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    for(i = 0 ; i < size+1 ; i++){
        waitpid(pid[i], &status, 0);
    }
}

void subshell(single_input input){
    int status;
    pid_t pid;
    parsed_input subshellInput;
    if(parse_line(input.data.subshell, &subshellInput)){
        int size = subshellInput.num_inputs;
        if(pid = fork()){
            waitpid(pid, &status, 0);
        }
        else{
            if(size == 1){
                singleInput(subshellInput.inputs[0]);
            }
            else{
                if(subshellInput.separator == SEPARATOR_PIPE){
                    pipedInput(subshellInput);
                } 
                else if(subshellInput.separator == SEPARATOR_SEQ){
                    sequentialInput(subshellInput);
                } 
                else if(subshellInput.separator == SEPARATOR_PARA){
                    // repeater(subshellInput);
                    parallelInput(subshellInput);
                }
            }
            exit(0);
        }
    }
}

void pipedSubshells(parsed_input input){
    int i, status;
    int size = input.num_inputs;
    pid_t pids[size];
    int pipes[size-1][2];
    for(i = 0 ; i < size ; i++){
        if(i == 0){
            pipe(pipes[i]);
            if(pids[i] = fork()){ // parent
                
            }
            else{
                close(pipes[i][0]);
                dup2(pipes[i][1], 1);
                close(pipes[i][1]);
                if(input.inputs[i].type == INPUT_TYPE_SUBSHELL){
                    subshell(input.inputs[i]);
                    exit(0);
                }
                else execvp(input.inputs[i].data.cmd.args[0], input.inputs[i].data.cmd.args);

            }
        }
        else if(i == size-1){
            if(pids[i] = fork()){
                int k;
                for(k = 0 ; k < i ; k++){
                    close(pipes[k][0]);
                    close(pipes[k][1]);
                }
                for(k = 0 ; k <= i ; k++){
                    waitpid(pids[k], &status, 0);
                }
            }
            else{
                int k;
                dup2(pipes[i-1][0], 0);
                for(k = 0 ; k < i ; k++){
                    close(pipes[k][0]);
                    close(pipes[k][1]);
                }
                if(input.inputs[i].type == INPUT_TYPE_SUBSHELL){
                    // subshell(input.inputs[i]);
                    int status;
                    pid_t pid;
                    parsed_input subshellInput;
                    if(parse_line(input.inputs[i].data.subshell, &subshellInput)){
                        int size = subshellInput.num_inputs;
                        if(pid = fork()){
                            waitpid(pid, &status, 0);
                        }
                        else{
                            if(size == 1){
                                singleInput(subshellInput.inputs[0]);
                            }
                            else{
                                if(subshellInput.separator == SEPARATOR_PIPE){
                                    pipedInput(subshellInput);
                                } 
                                else if(subshellInput.separator == SEPARATOR_SEQ){
                                    sequentialInput(subshellInput);
                                } 
                                else if(subshellInput.separator == SEPARATOR_PARA){
                                    repeater(subshellInput);
                                }
                            }
                            exit(0);
                        }
                    }
                    exit(0);
                }
                else execvp(input.inputs[i].data.cmd.args[0], input.inputs[i].data.cmd.args);
            }
        }
        else{
            pipe(pipes[i]);
            if(pids[i] = fork()){ // parent
                
            }
            else{
                int k;
                dup2(pipes[i-1][0], 0);
                dup2(pipes[i][1], 1);
                for(k = 0 ; k <= i ; k++){
                    close(pipes[k][0]);
                    close(pipes[k][1]);
                }
                if(input.inputs[i].type == INPUT_TYPE_SUBSHELL){
                    // subshell(input.inputs[i]);
                    int status;
                    pid_t pid;
                    parsed_input subshellInput;
                    if(parse_line(input.inputs[i].data.subshell, &subshellInput)){
                        int size = subshellInput.num_inputs;
                        if(pid = fork()){
                            waitpid(pid, &status, 0);
                        }
                        else{
                            if(size == 1){
                                singleInput(subshellInput.inputs[0]);
                            }
                            else{
                                if(subshellInput.separator == SEPARATOR_PIPE){
                                    pipedInput(subshellInput);
                                } 
                                else if(subshellInput.separator == SEPARATOR_SEQ){
                                    sequentialInput(subshellInput);
                                } 
                                else if(subshellInput.separator == SEPARATOR_PARA){
                                    repeater(subshellInput);
                                }
                            }
                            exit(0);
                        }
                    }
                    exit(0);
                }
                else execvp(input.inputs[i].data.cmd.args[0], input.inputs[i].data.cmd.args);
            }
        }
    }
}

bool strCompare(char* str1, const char str2[]){
    int i=0;
    while(str2[i]){
        if(str1[i] == str2[i]) i++;
        else return false;
    }
    return true;
}

int main(){

        parsed_input input;
        char line[1000];
        bool flag;
        
        while(1){
            printf("/> ");
            fgets(line, sizeof(line), stdin);

            if(strCompare(line, "quit")) break;

            if(parse_line(line, &input)){
                int size = input.num_inputs;
                if(size == 1){
                    if(input.inputs[0].type == INPUT_TYPE_SUBSHELL){
                        subshell(input.inputs[0]);
                    }
                    else{
                        singleInput(input.inputs[0]);
                    }
                }
                else{
                    if(input.separator == SEPARATOR_PIPE) pipedSubshells(input);
                    else if(input.separator == SEPARATOR_SEQ) sequentialInput(input);
                    else if(input.separator == SEPARATOR_PARA) parallelInput(input);
                }
            }
            free_parsed_input(&input);
        }
    return 0;
}