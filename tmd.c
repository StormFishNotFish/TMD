#include "3rd-party/md4c/src/md4c.h"
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
/* For CSI Code see https://xtermjs.org/docs/api/vtfeatures/#csi
 *
 */
#define CLEANUP(rcode) do{munmap(md_string, md_size);return rcode;}while(0);
#define PRINT_BLOCKQUOTE() {/*fprintf(stderr, "%d", bq_level);*/\
    for(int i = 0; i < (bq_level); i++){ fprintf(extra, "| ");fflush(extra);} }//Should check before every line  
#define PRINT_RETURN() {\
    fprintf(extra, "\n"); stats |= STAT_NEWLINE;\
}
#define PRINT_TAB() {\
    for(int i = 0; i < ul_level; i++) fprintf(extra, "\t");\
}
#define STAT_NEWLINE 0x00000001
#define STAT_NEWBQLVL 0x00000002
struct winsize winsiz;
uint32_t ul_level;
uint32_t ol_level;
uint32_t bq_level;//level of blockquote
uint32_t stats;
const MD_CHAR* ul_chars[] = {
    "•",  
    "◦", 
    "▪" 
};

MD_CHAR *md_string = NULL;
MD_SIZE md_size = 0;
/* UTF-8 chars holds three bytes maximum. For alignment, the last byte will fill as NUL char.
 * Also, it helps stdlib spliting the chars(also me).
 * Usage: ul_chars[uchar_index * 4]
 * About the bullets:
 * if ul_level is 1, the bullet is the round solid one
 * if ul_level is 2, the bullet is the round outline one
 * else the bullet is the square one
 */
int enter_block(MD_BLOCKTYPE type, void* detail, void* extra/*extra is stdout*/) {//Block element unsupported
    switch(type) {
        case MD_BLOCK_H:
            break;
        case MD_BLOCK_QUOTE:
            bq_level++;
            if(!(stats & STAT_NEWLINE))fprintf(extra, "| ");
            /* When you're in line 2, line one triggled new line. Then you print one quote and add the level. But then, when you print your text, you add one new blockquote which you have printed. That makes a unwanted quote.
             */
            break;
        case MD_BLOCK_UL:
            ul_level++;
            break;
        case MD_BLOCK_OL://TODO: ADD ORDER LIST MPL
            ul_level++;
            break;
        case MD_BLOCK_LI:
            if(!detail)CLEANUP(3);
            MD_BLOCK_LI_DETAIL *li_detail = (MD_BLOCK_LI_DETAIL*)detail;
            PRINT_BLOCKQUOTE();//LI must in a new line
            if(!li_detail->is_task) {//TODO:Add OL Impl
                PRINT_TAB();
                uint8_t bullet_index = (ul_level > 2 ? 2 : ul_level - 1);
                if(bullet_index < 0)CLEANUP(4);//Unexcepted Error!
                // fprintf(stderr, "[DEBUG]bullet_index: %d[DBGEND]", (int)bullet_index);
                fprintf(extra, "%s", ul_chars[bullet_index]);
                stats &= (~STAT_NEWLINE);
            }//TODO: ADD TASKLIST IMPL
            break;
        case MD_BLOCK_HR:
            break;
        case MD_BLOCK_P:
            break;
    }
    return 0;
}

int exit_block(MD_BLOCKTYPE type, void* detail, void* extra/*extra is stdout*/) {//Block element unsupported
    switch(type) {
        case MD_BLOCK_H:
            // fprintf(extra, "\n");
        case MD_BLOCK_QUOTE:
            bq_level--;
            break;
        case MD_BLOCK_UL:
            ul_level--;
            break;
        case MD_BLOCK_OL://TODO: ADD ORDER LIST IMPL
            ul_level--;
            break;
        case MD_BLOCK_LI:
            PRINT_RETURN(); 
            break;
        case MD_BLOCK_HR:
             
            break;
        case MD_BLOCK_P:
            PRINT_RETURN();
            break;
    }
    return 0;
}
//Triggled when exiting span elements
int exit_span(MD_SPANTYPE type, void* detail, void* extra) {
    switch(type) {
        case MD_SPAN_EM:
            fprintf(extra, "\033[23m");
            break;
        case MD_SPAN_STRONG:
            fprintf(extra, "\033[22m");
            break;
        case MD_SPAN_CODE:
            fprintf(extra, "\033[27m");
            break;
        case MD_SPAN_DEL:
            fprintf(extra, "\033[29m");
            break;
        case MD_SPAN_U:
            fprintf(extra, "\033[24m");
            break;
        case MD_SPAN_MARK:
            fprintf(extra, "\033[49m");
            break;
        case MD_SPAN_A:
            fprintf(extra, "\033]8;;\033\\");
            break;
    }
    return 0;
}
//Triggled when entering span elements
int enter_span(MD_SPANTYPE type, void* detail, void* extra) {
    int fd = (int)(intptr_t)extra;
    switch(type) {
        case MD_SPAN_EM:
            fprintf(extra, "\033[3m");
            break;
        case MD_SPAN_STRONG:
            fprintf(extra, "\033[1m");
            break;
        case MD_SPAN_CODE:
            fprintf(extra, "\033[7m");
            break;
        case MD_SPAN_DEL:
            fprintf(extra, "\033[9m");
            break;
        case MD_SPAN_U:
            fprintf(extra, "\033[4m");
            break;
        case MD_SPAN_MARK:
            fprintf(extra, "\033[104m");
            break;
        case MD_SPAN_A:
            if(!detail)CLEANUP(3);
            MD_SPAN_A_DETAIL *a_detail = (MD_SPAN_A_DETAIL*)detail;
            fprintf(extra, "\033]8;;%.*s\033\\", a_detail->href.size, a_detail->href.text);//OSC 8
            break;
    }
    return 0;
}
//
// void debug_log(const char* msg, void* extra) {
//     fprintf(stderr, "[DEBUG]: %s", msg);
// }
//Print normal text
int text_cb(MD_TEXTTYPE type, const MD_CHAR* text, MD_SIZE size, void* extra) {

    // fprintf(stderr, "\033[31m[DEBUG TEXT] Type: %d, Content: '%.*s'[DEBUG TEXT]\033[39m", type, (int)size, text);
    switch(type){
        case MD_TEXT_BR:
            PRINT_RETURN();
            break;
        case MD_TEXT_SOFTBR:
            fprintf(extra, " ");
            break;
        default:
            if(stats & STAT_NEWLINE) {
                PRINT_TAB();
                fprintf(extra, " ");//place hold for the bullet.
                PRINT_BLOCKQUOTE();
                stats &= (~STAT_NEWLINE);
            }
            fprintf(extra, "%.*s", size, text); 
            break;
    }
    fflush(extra);
    return 0;
}

void readfile(const char* filename, MD_SIZE *outsize, char** outtext) {
    int fd = open(filename, O_RDONLY);
    if(fd == -1) {
        perror("open");
        return;
    }
    struct stat sb;
    if(fstat(fd, &sb) == -1) {
        perror("fstat");
        close(fd);
        return;
    }
    if(sb.st_size == 0) {
        printf("readfile: File is empty");
        close(fd);
        *outsize = 0;
        return;
    }
    char* addr = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    if(addr == MAP_FAILED) {
        perror("mmap");
        return;
    }
    *outsize = sb.st_size;
    *outtext = addr;
}

int main(int argc, char** argv) {
    if(argc != 2) {
        printf("Usage: %s <filename.md>", argv[0]);
        return 1;
    }

    MD_CHAR *md_string = NULL;
    MD_SIZE md_size = 0;
    readfile(argv[1], &md_size, &md_string);
    if(md_string == NULL || md_size == 0) {
        printf("Error occurred in readfile\n");
        return 2;
    }
    
    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsiz) == -1) {
        perror("ioctl");
        CLEANUP(5);   
    }

    int flags = MD_FLAG_STRIKETHROUGH | MD_FLAG_UNDERLINE | MD_FLAG_HIGHLIGHT; 
    MD_PARSER parser = {
        .enter_block = enter_block,
        .leave_block = exit_block,
        .enter_span = enter_span,
        .leave_span = exit_span,
        // .debug_log = debug_log,
        .text = text_cb,
        .flags = flags
    };

    md_parse(md_string, md_size, &parser, stdout);
    CLEANUP(0);
}
