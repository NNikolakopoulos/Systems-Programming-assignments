
#ifndef HEADER_FILE
#define HEADER_FILE


typedef struct recordtag {
    char *citizenID;
    char *firstname;
    char *lastname;
    char *country;
    char age;
    char *virusname;
    bool vaccinated;
    char *dateVaccinated;
}Record;

typedef struct listNodeTag{
	struct listNodeTag *next;		
	void *item;	
}Listnode;

typedef struct listTag {
	Listnode *head;				
	unsigned int size;					
}List;

typedef struct hashTag {
    List **arr;
    List *countryList;
    List *filesParsed;
    unsigned int size, count;
    double loadfactor;
    bool *changed;
}HThash;

// ~~~~~~~~~~~~~ BLOOM FILTERT ~~~~~~~~~~~

typedef struct bloomfTag {
    unsigned long size;
    unsigned int hashfunctions;
    uint8_t *arr;
    char *virusName;
}bloomf;

//~~~~~~~~~~~~  SKIP LIST ~~~~~~~~~~~~~~~~~

typedef struct skipListNodeTag {
    char *key;
    void *item;
    unsigned int level;
    struct skipListNodeTag **arr;
}skipListNode;

typedef struct skipListTag {
    unsigned int level;
    unsigned int maxLevel;
    unsigned int size;
    skipListNode *head;
}skipList;


// ~~~~~~~~ List Node for countries ~~~~~~~~~~

typedef struct countryNode {
    char *country;
    List *viruses;
}countryNode;

typedef struct countryVirusNode {
    char *virusname;
    skipList *citizens;
}countryVirusNode;


// ~~~~~~~~  struct for request stats    ~~~~~~~~~

typedef struct req_statTag {
    char *date;
    bool accepted;
}req_stat;
//~~~~~~~~~~  VIRUS LIST ~~~~~~~~~~~~~~

typedef struct virusListNodeTag {
    char *virusName;
    List *req_stats;
    skipList *vaccinated_persons;
    skipList *not_vaccinated_persons;
    bloomf *bloom;
    struct virusListNodeTag *next;
}virusListNode;

typedef struct virusListTag {
    virusListNode *head;
    unsigned int size;
    unsigned long bloomsize;
    unsigned long pipe_buf_size;
    char *input_dir;
}virusList;


// ~~~~~~~~~~~~~~~ node for struct of parent ~~~~~~~~~~~~~~~~~

typedef struct virusParentNodeTag {
    char *country;
    virusList *vlist;
}virusParentNode;

// ~~~~~~~~~~~ node for list used in monitor processes to print logs ~~~~~~~~~~~

typedef struct globalDataTag {
    long int offset, requests, accepted, rejected;
    pid_t pid;
    char **countries;
    int countries_size;
    unsigned long pipe_buf_size;
}globalData;

// ~~~~~~~~~~~~~~ global struct for monitor process ~~~~~~~~~~~

typedef struct globalMonitorTag{
    List globalDataList;
}globalMonitor;

// ~~~~~~~~~~~ cyclic buffer struct ~~~~~~~~~~

typedef struct cyclicBufTag {
    char **paths;
    unsigned int size;
    unsigned int first, last;
    unsigned int numElements;
}cyclicBuf;

//~~~~~~~~ struct for thread arguments parsed while creation ~~~~~~~~~~

typedef struct threadArgsTag {
    HThash hash;
    virusList *list;
}threadArgs;


#endif