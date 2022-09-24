#include "types.h"

void vaccineStatusBloom(virusList *list, char *citizenid, char *virusname);
void vaccineStatus(virusList *list, char *citizenid, char *virusname);
void vaccineStatusAll(virusList *list, char *citizenid);
void populationStatus(HThash hash, virusList *list, char *virusname, char *date1, char *date2, char *country);
void popStatusByAge(HThash hash, virusList *list, char *virusname, char *date1, char *date2, char *country);
void insertCitizenRecord(HThash hash, virusList *list, char *citizenid, char *firstname, char *lastname, char *country, int age, char *virusname, char *vaccinated, char *date);
void listnonVaccinatedPersons(virusList *list, char *virusname);
void vaccinateNow(HThash hash, virusList *list, char *citizenid, char *fname, char *lname, char *country, int age, char *virusname);