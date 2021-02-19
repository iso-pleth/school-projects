#include "word_count.h"


int 
main(int argc, char *argv[]) {

    //parse args, check for bad_args errors
    int bad_args = FALSE;
    int sort = FALSE;
    int infile_arg_found = FALSE;
    char* filepath; 

    for (int i=0;i<argc;i++){
        if(strcmp("--infile",argv[i])==0){
            infile_arg_found = TRUE;
            if (i+1 < argc){
                if(strcmp(argv[i+1],"--sort")==0) bad_args = TRUE;
                filepath = argv[i+1];
            }
            else bad_args = TRUE;
        }
        if (strcmp(argv[i],"--sort")==0) sort = TRUE;
    }
    if (!infile_arg_found) bad_args = TRUE;

    if (bad_args){
        fprintf(stderr,"program: missing '--infile <filename> [--sort] [--print-words]'\n");
        exit(BAD_ARGS);
    }

    word_histogram* histogram = new_word_histogram();

    //read input file
    //use getline() to read file
    FILE* fp;
    char* line = NULL;
    size_t line_len = 0;

    fp = fopen(filepath,"r");
    if (fp==NULL) {
        fprintf(stderr,"unable to open '%s' for reading\n",filepath);
        exit(BAD_INFILE);   
    }
    while(getline(&line, &line_len, fp) != -1){
        tokenize_line(line,histogram);
    }
    free(line);
    fclose(fp);

    //print the resulting histogram
    print_histogram(histogram,sort);

    //free all node memory
    free_histogram(histogram);

    return 0;


}
