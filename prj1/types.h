




#ifndef HEADER_FILE
#define HEADER_FILE

typedef char *HTkey;

typedef void * HTitem;

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
    unsigned int size, count;
    double loadfactor;
}HThash;

// ~~~~~~~~ List Node for countries ~~~~~~~~~~

typedef struct countryNode {
    char *country;
    List *citizens;
}countryNode;


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

//~~~~~~~~~~  VIRUS LIST ~~~~~~~~~~~~~~

typedef struct virusListNodeTag {
    char *virusName;
    skipList *vaccinated_persons;
    skipList *not_vaccinated_persons;
    bloomf *bloom;
    struct virusListNodeTag *next;
}virusListNode;

typedef struct virusListTag {
    virusListNode *head;
    unsigned int size;
}virusList;




#endif