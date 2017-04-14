#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

typedef int bool;
enum { false = 0, true };

typedef struct word_adress_info
{
    char* file_name;
    int number_of_words;
    struct word_adress_info* next;
} FILE_INFO;
typedef struct word_info
{
    char* word_name;
    int total_number;
    int depth;
    struct word_adress_info* file_name_list;
    struct word_info* left;
    struct word_info* right;
} WORD;
/*opens the given path. returns file variable*/
FILE *file_operation(char* path, char* mode)
{
       FILE *fp;
       fp = fopen(path, mode);
       return fp;
}
/*joins path names*/
char* join_path_name(char *path, char *name)
{
    int size = strlen(path) + strlen(name) + 1;
    char* pathname = (char*)malloc(sizeof(char)*size);
    sprintf( pathname, "%s/%s", path, name);

    return pathname;
}
/*converts given word's letters to lowercase*/
char* word_to_lower(char* word)
{
    int i;
    for(i = 0; i<strlen(word); i++){
        word[i] = tolower(word[i]);
    }
    return word;
}
/*its string tokenizer function, mingw doesn't include this function, so i take it from internet because its public domain */
char* strtok_r(char *str, const char *delim, char **nextp)
{
    char *ret;

    if (str == NULL)
    {
        str = *nextp;
    }

    str += strspn(str, delim);

    if (*str == '\0')
    {
        return NULL;
    }

    ret = str;

    str += strcspn(str, delim);

    if (*str)
    {
        *str++ = '\0';
    }

    *nextp = str;

    return ret;
}
/*gets file name from path*/
char *get_file_name (char *path)
{

    char* token,*temp,*last_token,*sptr;
    temp = path;
    token = strtok_r (temp,"/",&sptr);
    while(token !=NULL)
    {
        last_token = token;
        token = strtok_r(NULL,"/",&sptr);
    }
    return last_token;
}
/*compares words,returns result*/
int word_compare(char* word1,char* word2)
{

    int result = strcmp(word1, word2);
    return result;
}
/*its dynamic file reader that i wrote, includes mode for reading regular text file and word-text file*/
char* read_txt_file(FILE* txt_file,int mode)
{
    int c;
    int nch = 0;
    int size = 10;
    char* data = (char*)malloc(size);
    while((c = getc(txt_file)) != EOF)
    {
        if(nch >= size-1)
        {
            size += 10;
            data = (char*)realloc(data,size);
        }
        if(mode == 0)
            data[nch++] = c;
        else if(mode == 1)
        {
            if(c == '-' || c == '\n' || c == '\r' || c == '\t' || c == ' ')
                data[nch++] = c;
            else if(isalnum(c))
                data[nch++] = c;
        }

    }
    data[nch++] = '\0';
    close(txt_file);
    return data;
}
/*adds word's file adress to bst node*/
void add_address_to_list(FILE_INFO** list, char *file_name)
{
    FILE_INFO* new_node, *temp, *iter;

    new_node = (FILE_INFO*)malloc(sizeof(FILE_INFO));
    new_node->file_name = (char*)malloc(1 + strlen(file_name));
    strcpy(new_node->file_name,file_name);
    new_node->number_of_words = 1;
    new_node->next = NULL;

    iter = *list;

    while(iter !=NULL)
    {
        if(word_compare(iter->file_name,file_name) == 0)
        {
            iter->number_of_words++;
            free(new_node);
            return;
        }
        iter = iter->next;
    }

    temp = *list;
    if (temp != NULL)
	{
		while (temp->next != NULL)
		{
			temp = temp->next;
		}
		temp->next = new_node;
	}
	else {
		*list = new_node;
	}

}
/*adds node to bst*/
void add_word_to_tree(WORD** tree, char* word, char* file_name,int* depth)
{
    WORD* temp = NULL;

    /*if tree node is empty*/
    if ((*tree) == NULL)
    {
        /*node created*/
        temp= (WORD *)malloc(sizeof(WORD));
        temp -> left = NULL;
        temp -> right = NULL;
        temp -> word_name = (char*)malloc(strlen(word+1));
        strcpy(temp -> word_name, word);
        temp -> total_number = 1;
        temp -> depth = *depth;
        temp ->file_name_list =NULL;
        add_address_to_list(&temp->file_name_list,file_name);
        *tree = temp;
        *depth=1;
        return;
    }

	if(word_compare((*tree)->word_name,word) == 0)
    {
        /*same word*/
        (*tree)->total_number++;
        (*depth) = 1;
        add_address_to_list(&(*tree)->file_name_list,file_name);
    }
    else if(word_compare((*tree)->word_name,word) < 0)
    {
        /*word tree < incoming word*/
        (*depth)++;
        add_word_to_tree(&(*tree)->right, word, file_name, &(*depth));
    }
    else if(word_compare((*tree)->word_name,word) > 0)
    {
        /*word tree > incoming word*/
        (*depth)++;
        add_word_to_tree(&(*tree)->left, word, file_name, &(*depth));
    }


}
/*enter function of bst*/
void create_bst(char* data, char *file_name,WORD** tree)
{
    char* word,*svptr;
    int depth = 1;
    word = strtok_r(data," \n\r\t",&svptr);
    while(word != NULL)
    {
        word = word_to_lower(word);
        add_word_to_tree(&(*tree),word,file_name,&depth);
        word = strtok_r(NULL," \n\r\t",&svptr);
    }
    free(data);
}
/*checks given file is ascii text or not, controls first 100 character*/
int check_file(FILE* txt_file)
{

    int c,count;
    int result = 1;
    while((c = getc(txt_file)) != EOF)
    {
        if(isascii(c) == 0){
            result = -1;
            break;
        }
        else if(count == 100)
            break;
        count++;
    }
    close(txt_file);
    return result;
}
/*checks given file is directory or not*/
int is_dir (char * filename)
{
    struct stat buf;
    int ret = stat (filename, & buf);
    if (0 == ret)
    {
        if ((buf.st_mode & S_IFMT) == S_IFDIR)
            return 0;
        else
            return 1;
    }
        return -1;
}
/*sends given path to read function and create bst function*/
void send_file_to_txt(char* path, char* file_name,WORD** tree)
{
    FILE* txt_file = file_operation(path,"r");
    char* txt_data;
    if(check_file(txt_file) != -1)
    {
        rewind(txt_file);
        txt_data = read_txt_file(txt_file,1);
        create_bst(txt_data,file_name,&(*tree));
    }


}
/*searches given (argv[1]) path and opens everything recursively. if its file, it sends to bst function*/
void search_dir(char *path_main,WORD** tree)
{
    DIR* dir;
    struct dirent* entry;

    char* file_name;


    if (!(dir = opendir(path_main)))
        return;
    if (!(entry = readdir(dir)))
        return;

    while ((entry = readdir(dir)) != NULL)
    {
        if (is_dir(path_main) == 0) {
            char *path = malloc(strlen(path_main) + strlen(entry->d_name) + 2);
            strcpy (path, path_main);
            strcat (path, "/");
            strcat (path, entry->d_name);

            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            if(is_dir(path) == 1)
                send_file_to_txt(path,entry->d_name,&(*tree));
            search_dir(path,&(*tree));

        }

    }
    closedir(dir);
}
/*prints one word's info.basic print for print commands*/
void print_word(WORD* tree,FILE* output)
{
    printf("%s\n",tree->word_name);
    fprintf(output,"%s\n",tree->word_name);

    printf("\ttotal number=%d\n\tdepth=%d\n",tree->total_number,tree->depth);
    fprintf(output,"\ttotal number=%d\n\tdepth=%d\n",tree->total_number,tree->depth);
    FILE_INFO* tmp = tree->file_name_list;
    if(tree->file_name_list == NULL)
        printf("\tfilenamelist null\n");
    while(tmp != NULL)
        {
            printf("\t%s %d\n",tmp->file_name,tmp->number_of_words);
            fprintf(output,"\t%s %d\n",tmp->file_name,tmp->number_of_words);
            tmp = tmp->next;
        }

}
/*Print all words in alphabetically descending order*/
void print_dsc(WORD* tree,FILE* output)
{
    if (tree)
    {
        print_dsc(tree->right, output);
        print_word(tree, output);
        print_dsc(tree->left, output);
    }
}
/*Print all words in alphabetically ascending order*/
void print_asc(WORD* tree,FILE* output)
{
    if (tree)
    {
        print_asc(tree->left, output);
        print_word(tree, output);
        print_asc(tree->right, output);
    }
}
/*Print all words pre-order. for PRINT TREE command */
void print_preorder(WORD * tree,FILE* output)
{
    if (tree)
    {

        print_word(tree, output);
        print_preorder(tree->left, output);
        print_preorder(tree->right, output);
    }

}
/*searches given word in tree*/
WORD* search_word(WORD** tree, char* word_name)
{
    if((*tree) == NULL)
    {
        return NULL;
    }

    if(word_compare((*tree)->word_name,word_name) > 0)
    {
        search_word(&((*tree)->left), word_name);
    }
    else if(word_compare((*tree)->word_name,word_name) < 0)
    {
        search_word(&((*tree)->right), word_name);
    }
    else if(word_compare((*tree)->word_name,word_name) == 0)
    {
        return *tree;
    }
}
/*returns max value of bst*/
WORD* max_word(WORD* tree){

	while(tree->right != NULL)
		tree = tree->right;
	return tree;
}
/*returns min value of bst*/
WORD* min_word(WORD* tree){
	while(tree->left != NULL)
		tree = tree->left;
	return tree;
}
/*decreases depth of tree*/
void decrease_depth (WORD* tree)
{
    if(tree)
    {
        tree->depth--;
        decrease_depth(tree->left);
        decrease_depth(tree->right);
    }
}
/*deletes given word from bst*/
WORD* delete_word(WORD* tree, char* word_name)
{
    if(tree == NULL)
        return tree;
    else if(word_compare(word_name,tree->word_name) < 0)
        tree->left = delete_word(tree->left, word_name);
    else if(word_compare(word_name,tree->word_name) > 0)
        tree->right = delete_word(tree->right, word_name);
    else
    {
        if(tree->left == NULL && tree->right == NULL)
        {
            free(tree);
            tree =NULL;
        }
        else if(tree->left == NULL)
        {
            WORD* temp = tree;
            tree = tree->right;
            decrease_depth(tree);
            free(temp);
        }
        else if(tree->right == NULL)
        {
            WORD* temp = tree;
            tree = tree->left;
            decrease_depth(tree);
            free(temp);
        }
        else
        {
            WORD* temp = min_word(tree->right);
            free(tree->word_name);
            tree->word_name = (char*)malloc(sizeof(temp->word_name));
            strcpy(tree->word_name, temp->word_name);
            tree->total_number = temp->total_number;
            free(tree->file_name_list);
            tree->file_name_list = temp->file_name_list;
            tree->right = delete_word(tree->right, temp->word_name);
        }
    }
    return tree;
}

/*after parsing path this function parses given input file and calls every command*/
void parse_and_create_command(char* input_path, char* command_txt_file,WORD** tree)
{
    FILE* command_file = file_operation(command_txt_file,"r");
    FILE* output = file_operation("output.txt","w");
    FILE* added_file, *control;
    char* input_list = read_txt_file(command_file,0);
    char* saveptr1,*saveptr2;
    char* line, *txt_data, *path;

    line = strtok_r(input_list,"\r\n",&saveptr1);
    while(line != NULL)
    {
        printf("%s\n",line);
        fprintf(output,"%s\n",line);

        if(!strcmp(line,"PRINT TREE ASC"))
        {
            print_asc(*tree,output);
        }
        else if(!strcmp(line,"PRINT TREE DSC"))
        {
            print_dsc(*tree,output);
        }
        else if(!strcmp(line,"PRINT TREE"))
        {
            print_preorder(*tree,output);
        }
        char* command = strtok_r(line," ",&saveptr2);

        if(!strcmp(command,"SEARCH"))
        {
            command = strtok_r(NULL," ",&saveptr2);
            command = word_to_lower(command);
            WORD* tmp = search_word(&(*tree),command);
            if(tmp)
                print_word(tmp,output);
            else
            {
                printf("Word not found in tree.\n");
                fprintf(output,"Word not found in tree.\n");
            }
        }
        else if(!strcmp(command,"ADD"))
        {
            command = strtok_r(NULL," ",&saveptr2);
            added_file = file_operation(command,"r");
            if(check_file(added_file) != -1)
            {
                rewind(added_file);
                txt_data = read_txt_file(added_file,1);
                path = get_file_name(command);
                create_bst(txt_data,path,&(*tree));

            }
            else
            {
                printf("This is not ascii text file!");
                fprintf(output,"This is not ascii text file!");
            }
        }
        else if(!strcmp(command,"REMOVE"))
        {
            command = strtok_r(NULL," ",&saveptr2);
            command = word_to_lower(command);
            *tree = delete_word(*tree,command);
        }


        line = strtok_r(NULL,"\n\r",&saveptr1);
    }
    close(command_file);


}
int main(int argc, char *argv[])
{

    WORD* tree = NULL;
    search_dir(argv[1],&tree);
    if(argc > 2)
    {
        parse_and_create_command(argv[1],argv[2],&tree);
    }
    else{
        printf("Usage: binarysearcher [input_path] [command_file]\n");
        return 0;
    }

    return 0;
}
